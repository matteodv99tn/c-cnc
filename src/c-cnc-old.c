
#include "defines.h"

#include "machine.h"
#include "program.h"
#include "point.h"


#define eprintf(...) fprintf(stderr, __VA_ARGS__);


int main(int argc, char *argv[]){

  point_t   *sp = NULL; // set point
  block_t   *b  = NULL; // current block
  program_t *p  = NULL; // program

  data_t    t, tq, lambda, f; // parameters for the machine

  machine_t *machine = machine_new("settings.ini");
  if(!machine){
    eprintf("Error creating machine istance\n");
    exit(EXIT_FAILURE);
  }

  tq  = machine_tq(machine);  // Quantization time
  p   = program_new(argv[1]); // Name of program as parameter of input progrma
  if(!p){
    eprintf("Error: could not create program, exiting\n");
    exit(EXIT_FAILURE);
  }

  if(program_parse(p, machine) == EXIT_FAILURE){
    eprintf("Could not parse program in \"%s\"; exiting\n", argv[1]);
    exit(EXIT_FAILURE);
  } 
  
  program_print(p, stderr);


  //   __  __       _         _                   
  //  |  \/  | __ _(_)_ __   | | ___   ___  _ __  
  //  | |\/| |/ _` | | '_ \  | |/ _ \ / _ \| '_ \ 
  //  | |  | | (_| | | | | | | | (_) | (_) | |_) |
  //  |_|  |_|\__,_|_|_| |_| |_|\___/ \___/| .__/ 
  //                                       |_|    
  while((b = program_next(p))){

    if(block_type(b) == RAPID || block_type(b) > ARC_CCW)
      continue; // Stop processing and go to next iteration
    
    eprintf("Interpolating block \"%s\"\n", block_line(b));

    // interpolation loop
    for(t = 0; t <= block_dt(b); t += tq){
      
      lambda  = block_lambda(b, t, &f);       // f: current feedrate
      sp      = block_interpolate(b, lambda);

      if(!sp) continue;

      printf("%lu,%f,%f,%f,%f,%f,%f,%f\n", block_n(b), 
        t, lambda, lambda*block_length(b), f,
        point_x(sp), point_y(sp), point_z(sp) );

    }

  }


  machine_free(machine);
  program_free(p);

  return EXIT_SUCCESS;
}