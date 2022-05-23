//   __  __            _     _            
//  |  \/  | __ _  ___| |__ (_)_ __   ___ 
//  | |\/| |/ _` |/ __| '_ \| | '_ \ / _ \
//  | |  | | (_| | (__| | | | | | | |  __/
//  |_|  |_|\__,_|\___|_| |_|_|_| |_|\___|
//
#include "machine.h"
#include "inic.h"
#include <mosquitto.h>
#include <mqtt_protocol.h>
#include <unistd.h>

//   ____            _                 _   _                 
//  |  _ \  ___  ___| | __ _ _ __ __ _| |_(_) ___  _ __  ___ 
//  | | | |/ _ \/ __| |/ _` | '__/ _` | __| |/ _ \| '_ \/ __|
//  | |_| |  __/ (__| | (_| | | | (_| | |_| | (_) | | | \__ \
//  |____/ \___|\___|_|\__,_|_|  \__,_|\__|_|\___/|_| |_|___/

#define BUFLEN 1024

typedef struct machine {
  data_t A, tq, max_error, error; // error = current error
  point_t *zero, *offset, *setpoint, *position; // position = actual position 
  char broker_address[BUFLEN];
  int broker_port;
  char pub_topic[BUFLEN];
  char sub_topic[BUFLEN];
  char pub_buffer[BUFLEN];
  struct mosquitto *mqt;
  struct mosquitto_message *msg;
  int connecting;
} machine_t;

// callbacks
static void on_connect(struct mosquitto *mqt, void *obj, int rc);
static void on_message(struct mosquitto *mqt, void *ud, const struct mosquitto_message *msg);

//   _____                 _   _                 
//  |  ___|   _ _ __   ___| |_(_) ___  _ __  ___ 
//  | |_ | | | | '_ \ / __| __| |/ _ \| '_ \/ __|
//  |  _|| |_| | | | | (__| |_| | (_) | | | \__ \
//  |_|   \__,_|_| |_|\___|\__|_|\___/|_| |_|___/
                                              
// LIFECYCLE ===================================================================

// Create a new instance reading data from an INI file
// If the INI file is not given (NULL), provide sensible default values
machine_t *machine_new(const char *ini_path) {
  machine_t *m = (machine_t *)calloc(1, sizeof(machine_t));
  if (!m) {
    perror("Error creating machine object");
    exit(EXIT_FAILURE);
  }
  if (ini_path) { // load values from INI file
    void *ini = ini_init(ini_path);
    data_t x, y, z;
    int rc = 0;
    if (!ini) {
      fprintf(stderr, "Could not open the ini file %s\n", ini_path);
      return NULL;
    }
    rc += ini_get_double(ini, "C-CNC", "A", &m->A);
    rc += ini_get_double(ini, "C-CNC", "error", &m->max_error);
    rc += ini_get_double(ini, "C-CNC", "tq", &m->tq);
    rc += ini_get_double(ini, "C-CNC", "origin_x", &x);
    rc += ini_get_double(ini, "C-CNC", "origin_y", &y);
    rc += ini_get_double(ini, "C-CNC", "origin_z", &z);
    m->zero = point_new();
    point_set_xyz(m->zero, x, y, z);
    rc += ini_get_double(ini, "C-CNC", "offset_x", &x);
    rc += ini_get_double(ini, "C-CNC", "offset_y", &y);
    rc += ini_get_double(ini, "C-CNC", "offset_z", &z);
    m->offset = point_new();
    point_set_xyz(m->offset, x, y, z);
    ini_free(ini);
    if (rc > 0) {
      fprintf(stderr, "Missing/wrong %d config parameters\n", rc);
      return NULL;
    }
  }
  else { // provide default values
    m->A = 125;
    m->max_error = 0.005;
    m->tq = 0.005;
    m->zero = point_new();
    point_set_xyz(m->zero, 0, 0, 0);
    m->offset = point_new();
    point_set_xyz(m->offset, 0, 0, 0);
  }

  m->setpoint = point_new();
  m->position = point_new();
  point_modal(m->zero, m->setpoint);

  return m;
}

void machine_free(machine_t *m) {
  assert(m);
  point_free(m->zero);
  point_free(m->offset);
  point_free(m->setpoint);
  free(m);
  m = NULL;
}

// MQTT COMMUNICATIONS =========================================================

// return value is 0 on success
int machine_connect(machine_t *m, machine_on_message callback) {
  assert(m);
  m->mqt = mosquitto_new(NULL, 1, m);
  if (!m->mqt) {
    perror("Could not create MQTT");
    return 1;
  }
  mosquitto_connect_callback_set(m->mqt, on_connect);
  mosquitto_message_callback_set(m->mqt, callback ? callback : on_message);
  if (mosquitto_connect(m->mqt, m->broker_address, m->broker_port, 60) != MOSQ_ERR_SUCCESS) {
    perror("Could not connect to broker");
    return 2;
  }
  // wait for connection to establish
  while (m->connecting) {
    mosquitto_loop(m->mqt, -1, 1);
  }
  return 0;
}

int machine_sync(machine_t *m) {
  assert(m);

  if(mosquitto_loop(m->mqt, 0, 1) != MOSQ_ERR_SUCCESS){
    perror("mosquitto_loop error");
    return 1;
  }

  // Fill up pub_buffer with current set point
  snprintf(m->pub_buffer, BUFLEN, "%f,%f,%f", point_x(m->setpoint), point_y(m->setpoint), point_z(m->setpoint));

  // Send buffer over MQTT
  mosquitto_publish(m->mqt, NULL, m->pub_topic, strlen(m->pub_buffer), m->pub_buffer, 0, 0);
  // Note: for wired connection (as us) there is no problem at having a qos = 0; more complex network might require higher quality of service
  return 0;
}

int machine_listen_start(machine_t *m){
  assert(m);

  if(mosquitto_subscribe(m->mqt, NULL, m->sub_topic, 0) != MOSQ_ERR_SUCCESS){
    perror("Could not subscribe \n");
    return 1;
  }

  eprintf("Subscribed to topic %s \n", m->sub_topic);

  return 0;
}

int machine_listen_stop(machine_t *m){
  assert(m);

  if(mosquitto_unsubscribe(m->mqt, NULL, m->sub_topic) != MOSQ_ERR_SUCCESS){
    perror("Could not unsubscribe \n");
    return 1;
  }

  eprintf("Unsubscribed to topic %s \n", m->sub_topic);
  return 0;
}

void machine_listen_update(machine_t *m){
  if(mosquitto_loop(m->mqt, 0, 1) != MOSQ_ERR_SUCCESS){
    perror("mosquitto_loop error");
    return;
  }
}

void machine_disconnect(machine_t *m) {
  assert(m);

  // Check for pending messages to be sent
  while(mosquitto_want_write(m->mqt)) 
    usleep(10000);

  mosquitto_disconnect(m->mqt);
}



// ACCESSORS ===================================================================

#define machine_getter(typ, par) \
typ machine_##par(const machine_t *m) { assert(m); return m->par; }

machine_getter(data_t, A);
machine_getter(data_t, tq);
machine_getter(data_t, max_error);
machine_getter(data_t, error);
machine_getter(point_t *, zero);
machine_getter(point_t *, offset);
machine_getter(point_t *, setpoint);
machine_getter(point_t *, position);


// STATIC FUNCTIONS

static void on_connect(struct mosquitto *mqt, void *obj, int rc) {
  machine_t *m = (machine_t *)obj;
  // Successful connection
  if (rc == CONNACK_ACCEPTED) {
    eprintf("-> Connected to %s:%d\n", m->broker_address, m->broker_port);
    // subscribe
    if (mosquitto_subscribe(mqt, NULL, m->sub_topic, 0) != MOSQ_ERR_SUCCESS) {
      perror("Could not subscribe");
      exit(EXIT_FAILURE);
    }
  }
  // Failed to connect
  else {
    eprintf("-X Connection error: %s\n", mosquitto_connack_string(rc));
    exit(EXIT_FAILURE);
  }
  m->connecting = 0;
}

static void on_message(struct mosquitto *mqt, void *ud, const struct mosquitto_message *msg) {

  machine_t *m = (machine_t *)ud;
  char *subtopic = strrchr(msg->topic, '/') + 1; // last word in the MQTT topic

  eprintf("<- message: %s\n", (char *)msg->payload);
  mosquitto_message_copy(m->msg, msg);

  if(strcmp(subtopic, "error") == 0){

    m->error = atof(msg->payload);

  } else if (strcmp(subtopic, "position")) {
    // We have to parse a string in the form "123.4, 100.4, -1.421"
    // into 3 coordinate values x,y,z
    char *nxt = msg->payload;
    point_set_x(m->position, strtod(nxt, &nxt)  );
    point_set_y(m->position, strtod(nxt+1, &nxt));
    point_set_z(m->position, strtod(nxt+1, &nxt));
  }
  else {
    eprintf("Got unexpected message on %s!\n", subtopic);
  }

}