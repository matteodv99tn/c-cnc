/******************************************************************************
Finite State Machine
Project: C-CNC 2020
Description: Finite state machine for C-CNC

Generated by gv_fsm ruby gem, see https://rubygems.org/gems/gv_fsm
gv_fsm version 0.2.3
Generation date: 2020-09-01 17:39:38 +0200
Generated from: fsm2.dot
The finite state machine has:
  4 states
  2 transition functions
******************************************************************************/

#include <syslog.h>
#include "fsm.h"

//    ____      _ _ ____             _        
//   / ___|__ _| | | __ )  __ _  ___| | _____ 
//  | |   / _` | | |  _ \ / _` |/ __| |/ / __|
//  | |__| (_| | | | |_) | (_| | (__|   <\__ \
//   \____\__,_|_|_|____/ \__,_|\___|_|\_\___/

// block callback                                       
void print_block_descr(block_t *b, void*userdata) {
  struct machine *m = (struct machine *)userdata;
  // print block descrption on stderr
  fprintf(stderr, "\n");
  block_print(b, stderr);
  fprintf(stderr, "Offset: %9.3f %9.3f %9.3f\n", m->cfg->offset[0], m->cfg->offset[1], m->cfg->offset[2]);
  // print column header for data table on stdout at the
  // beginning of each g-code block
  printf("#n type t_time b_time error x y z mx my mz\n");
  wait_next(0); // reset internal time counter
}

// time loop callback
block_ctrl_t time_loop(block_t *b, data_t t, void *userdata) {
  struct machine *m = (struct machine *)userdata;
  point_t *position;
  data_t error;
  static data_t cur_time = 0; // static vars retain their value between calls
  data_t *offset = m->cfg->offset;

  // timing: keep track of passing time and wait until
  // system clock elapses the next multiple of tq
  cur_time += m->cfg->tq;
  wait_next(m->cfg->tq * 1E9);

  // deal with the block types
  switch(b->type) {
    case RAPID:
      position = &b->target;
      break;
    case LINE: {
      data_t l = block_lambda(b, t);
      point_t p = block_interpolate(b, l);
      position = &p;
      break;
    }
    case SET_OFFSET: {
      offset[0] += b->offset.x;
      offset[1] += b->offset.y;
      offset[2] += b->offset.z;
      return STOP;
    }
    case CLEAR_OFFSET: {
      offset[0] = 0.0;
      offset[1] = 0.0;
      offset[2] = 0.0;
      return STOP;
    }
    // motion types that are not implemented (yet): give warning
    // and move to the next g-code block
    case ARC_CW:
    case ARC_CCW:
      fprintf(stderr, "WARNING: arc interpolation is not yet implemented!\n");
      return STOP;
    // catch all condition
    default: {
      fprintf(stderr, "WARNING: unsupported block type: %s\n", b->line);
      return STOP;
    }
  }

  // communicate with the machine
  machine_go_to(m, position->x + offset[0], position->y + offset[1], position->z + offset[2]); // axes setpoints
  machine_do_step(m, m->cfg->tq);                          // forward integrate axes dynamics
  // get the position error (distance of actual position from nominal position)
  error = machine_error(m);
  
  // print out values
  fprintf(stdout, "%3d %1d %7.3f %7.3f %7.3f %8.3f %8.3f %8.3f %8.3f %8.3f %8.3f\n", b->n, b->type, cur_time, t, error, position->x, position->y, position->z, m->x->x, m->y->x, m->z->x);

  // a RAPID block stops when the error becomes smaller than a given threshold
  if ((b->type == RAPID) && (error <= m->cfg->error)) {
    return STOP;
  }

  // by default, continue the loop
  return CONTINUE;
}


// SEARCH FOR Your Code Here FOR CODE INSERTION POINTS!

// GLOBALS
// State human-readable names
const char *state_names[] = {"init", "idle", "run", "stop"};

// List of state functions
state_func_t *const state_table[NUM_STATES] = {
  do_init, // in state init
  do_idle, // in state idle
  do_run,  // in state run
  do_stop, // in state stop
};

// Table of transition functions
transition_func_t *const transition_table[NUM_STATES][NUM_STATES] = {
  /* states:     init        , idle        , run         , stop         */
  /* init    */ {NULL        , setup       , NULL        , NULL        }, 
  /* idle    */ {NULL        , NULL        , NULL        , idle_to_stop}, 
  /* run     */ {NULL        , setup       , NULL        , NULL        }, 
  /* stop    */ {NULL        , NULL        , NULL        , NULL        }, 
};

//  ____  _        _       
// / ___|| |_ __ _| |_ ___ 
// \___ \| __/ _` | __/ _ \
//  ___) | || (_| | ||  __/
// |____/ \__\__,_|\__\___|
//                         
//   __                  _   _                 
//  / _|_   _ _ __   ___| |_(_) ___  _ __  ___ 
// | |_| | | | '_ \ / __| __| |/ _ \| '_ \/ __|
// |  _| |_| | | | | (__| |_| | (_) | | | \__ \
// |_|  \__,_|_| |_|\___|\__|_|\___/|_| |_|___/
//                                             

// Function to be executed in state init
// valid return states: STATE_IDLE
state_t do_init(state_data_t *sd) {
  state_t next_state = STATE_IDLE;

  syslog(LOG_INFO, "[FSM] In state init");
  // initialize program and machine objects
  sd->p = program_new(sd->program_file);
  sd->m = machine_new(sd->config_path);
  sd->cfg = sd->m->cfg;

  // deal with the program
  program_parse(sd->p, sd->cfg);
  fprintf(stderr, "Parsed program from %s\n", sd->program_file);
  program_print(sd->p, stderr);

  // Prepare for run: enable the viewer
  machine_enable_viewer(sd->m, sd->viewer_path);
  sleep(1);
  machine_set_position(sd->m, sd->cfg->zero[0], sd->cfg->zero[1], sd->cfg->zero[2]);


  switch (next_state) {
    case STATE_IDLE:
      break;
    default:
      syslog(LOG_WARNING, "[FSM] Cannot pass from init to %s, remaining in this state", state_names[next_state]);
      next_state = NO_CHANGE;
  }
  return next_state;
}


// Function to be executed in state idle
// valid return states: NO_CHANGE, STATE_IDLE, STATE_RUN, STATE_STOP
state_t do_idle(state_data_t *sd) {
  state_t next_state = NO_CHANGE;
  char c;

  syslog(LOG_INFO, "[FSM] In state idle");
  fprintf(stderr, "Do you want to:\n 1. run %s\n 2. set offset to current position\n 3. show status\n 4. quit\nType 1-4: ", sd->program_file);
  c = getchar();
  fflush(stdin); // drop further characters in stream
  switch (c) {
  case '1':
    machine_set_position_from_viewer(sd->m);
    next_state = STATE_RUN;
    break;
  case '2':
    sd->m->cfg->offset[0] = sd->m->viewer->coord[0];
    sd->m->cfg->offset[1] = sd->m->viewer->coord[1];
    sd->m->cfg->offset[2] = sd->m->viewer->coord[2];
  case '3':
    machine_set_position_from_viewer(sd->m);
    fprintf(stderr, "Machine now at: %9.3f %9.3f %9.3f\n", sd->m->x->x, sd->m->y->x, sd->m->z->x);
    fprintf(stderr, "    velocities: %9.3f %9.3f %9.3f\n", sd->m->x->v, sd->m->y->v, sd->m->z->v);
    fprintf(stderr, "        viewer: %9.3f %9.3f %9.3f\n", sd->m->viewer->coord[0], sd->m->viewer->coord[1], sd->m->viewer->coord[2]);
    fprintf(stderr, "        offset: %9.3f %9.3f %9.3f\n", sd->m->cfg->offset[0], sd->m->cfg->offset[1], sd->m->cfg->offset[2]);
    break;
  case '4':
    next_state = STATE_STOP;
    break;
  }
  
  switch (next_state) {
    case NO_CHANGE:
    case STATE_IDLE:
    case STATE_RUN:
    case STATE_STOP:
      break;
    default:
      syslog(LOG_WARNING, "[FSM] Cannot pass from idle to %s, remaining in this state", state_names[next_state]);
      next_state = NO_CHANGE;
  }
  return next_state;
}


// Function to be executed in state run
// valid return states: STATE_IDLE
state_t do_run(state_data_t *sd) {
  state_t next_state = STATE_IDLE;

  syslog(LOG_INFO, "[FSM] In state run");
  program_loop(sd->p, time_loop, print_block_descr, sd->m);
  next_state = STATE_IDLE;
  
  switch (next_state) {
    case STATE_IDLE:
      break;
    default:
      syslog(LOG_WARNING, "[FSM] Cannot pass from run to %s, remaining in this state", state_names[next_state]);
      next_state = NO_CHANGE;
  }
  return next_state;
}


// Function to be executed in state stop
// valid return states: NO_CHANGE
state_t do_stop(state_data_t *sd) {
  state_t next_state = NO_CHANGE;

  syslog(LOG_INFO, "[FSM] In state stop");
  // release allocated memory
  program_free(sd->p);
  machine_free(sd->m);
  
  switch (next_state) {
    case NO_CHANGE:
      break;
    default:
      syslog(LOG_WARNING, "[FSM] Cannot pass from stop to %s, remaining in this state", state_names[next_state]);
      next_state = NO_CHANGE;
  }
  return next_state;
}


//  _____                    _ _   _              
// |_   _| __ __ _ _ __  ___(_) |_(_) ___  _ __   
//   | || '__/ _` | '_ \/ __| | __| |/ _ \| '_ \
//   | || | | (_| | | | \__ \ | |_| | (_) | | | | 
//   |_||_|  \__,_|_| |_|___/_|\__|_|\___/|_| |_| 
//                                                
//   __                  _   _                 
//  / _|_   _ _ __   ___| |_(_) ___  _ __  ___ 
// | |_| | | | '_ \ / __| __| |/ _ \| '_ \/ __|
// |  _| |_| | | | | (__| |_| | (_) | | | \__ \
// |_|  \__,_|_| |_|\___|\__|_|\___/|_| |_|___/
//    
                                         
// This function is called in 2 transitions:
// 1. from init to idle
// 2. from run to idle
void setup(state_data_t *sd) {
  syslog(LOG_INFO, "[FSM] State transition setup");
  machine_reset(sd->m);
}

// This function is called in 1 transition:
// 1. from idle to stop
void idle_to_stop(state_data_t *data) {
  syslog(LOG_INFO, "[FSM] State transition idle_to_stop");
  /* Your Code Here */
}


//  ____  _        _        
// / ___|| |_ __ _| |_ ___  
// \___ \| __/ _` | __/ _ \
//  ___) | || (_| | ||  __/ 
// |____/ \__\__,_|\__\___| 
//                          
//                                              
//  _ __ ___   __ _ _ __   __ _  __ _  ___ _ __ 
// | '_ ` _ \ / _` | '_ \ / _` |/ _` |/ _ \ '__|
// | | | | | | (_| | | | | (_| | (_| |  __/ |   
// |_| |_| |_|\__,_|_| |_|\__,_|\__, |\___|_|   
//                              |___/           

state_t run_state(state_t cur_state, state_data_t *data) {
  state_t new_state = state_table[cur_state](data);
  if (new_state == NO_CHANGE) new_state = cur_state;
  transition_func_t *transition = transition_table[cur_state][new_state];
  if (transition)
    transition(data);
  return new_state == NO_CHANGE ? cur_state : new_state;
};

#ifdef TEST_MAIN
#include <unistd.h>
int main() {
  state_t cur_state = STATE_INIT;
  openlog("SM", LOG_PID | LOG_PERROR, LOG_USER);
  syslog(LOG_INFO, "Starting SM");
  do {
    cur_state = run_state(cur_state, NULL);
    sleep(1);
  } while (cur_state != STATE_STOP);
  return 0;
}
#endif
