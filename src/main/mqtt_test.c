//   __  __  ___ _____ _____
//  |  \/  |/ _ \_   _|_   _|
//  | |\/| | | | || |   | |
//  | |  | | |_| || |   | |
//  |_|  |_|\__\_\|_|   |_|
// Simple MQTT client example
#include "../inic.h"
#include <mosquitto.h>
#include <mqtt_protocol.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>

//   ____            _                 _   _                 
//  |  _ \  ___  ___| | __ _ _ __ __ _| |_(_) ___  _ __  ___ 
//  | | | |/ _ \/ __| |/ _` | '__/ _` | __| |/ _ \| '_ \/ __|
//  | |_| |  __/ (__| | (_| | | | (_| | |_| | (_) | | | \__ \
//  |____/ \___|\___|_|\__,_|_|  \__,_|\__|_|\___/|_| |_|___/
// 
// Preprocessor macros and constants
#define eprintf(...) fprintf(stderr, __VA_ARGS__);
#define BUFLEN 1024
#define INI_FILE "settings.ini"
#define INI_SECTION "MQTT"

// Custom types
typedef struct {
  char  broker_addr[BUFLEN];      // broker address
  int   broker_port;              // broker port
  char  topic[BUFLEN];            // topic where data is published
  struct mosquitto_message *msg;  // Mosquitto message
} userdata_t;

// Functions
void on_connect     (struct mosquitto *mqt, void *ud, int rc); // rc: return code
void on_disconnect  (struct mosquitto *mqt, void *ud, int rc);
void on_subscribe   (struct mosquitto *mqt, void *ud, int mid /* message id */, int qos_len /*level of quality */, const int *qos); 
void on_unsubscribe (struct mosquitto *mqt, void *ud, int rc);
void on_message     (struct mosquitto *mqt, void *ud, const struct mosquitto_message *msg);
//                   _
//   _ __ ___   __ _(_)_ __
//  | '_ ` _ \ / _` | | '_ \
//  | | | | | | (_| | | | | |
//  |_| |_| |_|\__,_|_|_| |_|
//
int main(int argc, char const *argv[]) {
  
  void       *ini = NULL;
  int        delay = 0;
  struct     mosquitto *mqt;
  userdata_t ud = {
    .broker_addr = "localhost", 
    .broker_port = 1883,
    .topic = "ccnc/test" 
  }; 

  // INI file parsing
  ini = ini_init(INI_FILE);
  if(!ini){
    eprintf("Error opening INI file\n");
    return 1;
  }

  ini_get_char(ini, INI_SECTION, "broker_addr", ud.broker_addr, BUFLEN);
  ini_get_int (ini, INI_SECTION, "broker_port", &ud.broker_port);
  ini_get_char(ini, INI_SECTION, "topic",       ud.topic, BUFLEN);
  ini_get_int (ini, INI_SECTION, "delay", &delay);

  //   __  __  ___ _____ _____ 
  //  |  \/  |/ _ \_   _|_   _|
  //  | |\/| | | | || |   | |  
  //  | |  | | |_| || |   | |  
  //  |_|  |_|\__\_\|_|   |_|  
  // Initialization of the library
  if(mosquitto_lib_init() != MOSQ_ERR_SUCCESS){
    perror("Could not initialize MQTT library\n");
    return 2;
  }                       

  { // Verify library version
    int major, minor, revision;
    mosquitto_lib_version(&major, &minor, &revision);
    eprintf("Library version %d.%d.%d \n", major, minor, revision);
  }

  // Create the MQTT object
  mqt = mosquitto_new(NULL, 1, &ud);
  if(!mqt){
    perror("Could not create MQTT object\n");
    return 3;
  }

  // Callbacks
  mosquitto_connect_callback_set(mqt, on_connect);
  mosquitto_disconnect_callback_set(mqt, on_disconnect);
  mosquitto_subscribe_callback_set(mqt, on_subscribe);
  mosquitto_unsubscribe_callback_set(mqt, on_unsubscribe);
  mosquitto_message_callback_set(mqt, on_message);

  // Connect to the broker
  if(mosquitto_connect(mqt, ud.broker_addr, ud.broker_port, 60) != MOSQ_ERR_SUCCESS){
    perror("Error connecting to the broker\n");
    return 4;
  }

  // Main loop
  while(1){

    if(mosquitto_loop(mqt, -1, 1) != MOSQ_ERR_SUCCESS){
      perror("mosquitto_loop error");
      break;
    }

    usleep(delay * 1000); // note: delay is in microsecond 

  }

  // unsubscribe and disconnect
  mosquitto_unsubscribe(mqt, NULL, ud.topic);
  mosquitto_disconnect(mqt);
  mosquitto_destroy(mqt);
  mosquitto_lib_cleanup();


  return 0;
}

//   ____        __ _       _ _   _                 
//  |  _ \  ___ / _(_)_ __ (_) |_(_) ___  _ __  ___ 
//  | | | |/ _ \ |_| | '_ \| | __| |/ _ \| '_ \/ __|
//  | |_| |  __/  _| | | | | | |_| | (_) | | | \__ \
//  |____/ \___|_| |_|_| |_|_|\__|_|\___/|_| |_|___/
// 

void on_connect(struct mosquitto *mqt, void *obj, int rc){

  userdata_t *ud = (userdata_t *)obj;
  if(rc == CONNACK_ACCEPTED) {

    eprintf("-> Connected to %s:%d \n", ud->broker_addr, ud->broker_port);
    
    // subscription
    if(mosquitto_subscribe(mqt, NULL, ud->topic, 0) != MOSQ_ERR_SUCCESS){
      perror("Could not subscribe");
      exit(EXIT_FAILURE);
    }

  }
  else {
    eprintf("-X Connection error: %s \n", mosquitto_connack_string(rc));
    exit(EXIT_FAILURE);
  }
}

void on_disconnect(struct mosquitto *mqt, void *ud, int rc){

  eprintf("<- Disconnected\n");

}

void on_subscribe(struct mosquitto *mqt, void *ud, int mid, int qos_len, const int *qos){

  eprintf("-> Successfully subsbribed to topic %s\n", ((userdata_t *)ud)->topic);

}

void on_unsubscribe(struct mosquitto *mqt, void *ud, int rc){

  eprintf("<- Unsubscribed from topic %s\n", ((userdata_t *)ud)->topic);

}

void on_message(struct mosquitto *mqt, void *ud, const struct mosquitto_message *msg){

  eprintf("<- message: %s\n", msg->payload);
  // minuto 13

}