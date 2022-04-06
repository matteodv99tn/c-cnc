#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_BUFFER_SIZE 1024

typedef enum{
  ERROR_SUCCESS = 0,
  ERROR_ARGS,
  ERROR_READ,
  ERROR_WRITE
} error_t;

int main(const int argc, const char* argv[]){

  if(argc != 2){
    fprintf(stderr, "I need only 2 arguments!\n");
    return ERROR_ARGS;
  }

  const char* filename = argv[1];
  fprintf(stdout, "Trying to open %s\n", filename);

  FILE* f = fopen(filename, "w"); // Open a file in read-only mode
  if( !f ) {  // if( f == NULL)
    fprintf(stderr, "Cannot open the file! \n");
    return ERROR_WRITE;
  }

  fprintf(f, "First line\n");
  fprintf(f, "A new line uh?! \n");
  fprintf(f, "Last one :) \n");
  fclose(f);

  f = fopen(filename, "r");
  char line_buffer[MAX_BUFFER_SIZE];
  while(fgets(line_buffer, MAX_BUFFER_SIZE, f)){
    fprintf(stdout, "%s", line_buffer);
  }
  fclose(f);

  fprintf(stderr, "> %s <", line_buffer);

  return ERROR_SUCCESS;
}









