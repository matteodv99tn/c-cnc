
#include "include_guards.h"

int main(const int argc, const char* argv[]){

  print_args(argc, argv);
  
  if(argc == 1){
    printf("No enough arguments to proceed! Terminating. \n");
    return 1;
  }

  double a = atof(argv[1]); // Functions that converts *char to floating point

  printf("Received %f\n", a);

  return 0;
}

void print_int_array(const int* const ary, const int n){
  printf("[");
  for(int i = 0; i < n; i++)
    printf(" %d ", ary[i]);  
  printf("]\n");
}

void print_args(const int argc, const char* argv[]){

  printf("Program called with %d arguments: \n", argc);
  for(int i = 0; i < argc; i++)
    printf("%d. %s\n", i+1, argv[i]);

}