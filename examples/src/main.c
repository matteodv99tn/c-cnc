#include <stdio.h>
#include <stdlib.h>

#include "inout_pointers.h"

int main(){
  
  int n = 10; // size of the array

  /* --- FIRST EXAMPLE --- */
  /*
   * array of floats using a function that has as input
   * only the dimension and returns a pointer
   * 
   */
  printf("\n ====== 1st EXAMPLE ====== \n");
  float *a;
  printf("Initial address of a is %p\n", a);
  a = memory_allocator_1(n);
  print_array(a, n);
  printf("Address of a after initialization is %p\n", a);

  /* --- SECOND EXAMPLE --- */
  /*
   * array of floats using a void function that has as input
   * both the dimension and the pointer
   * 
   */
  printf("\n ====== 2nd EXAMPLE ====== \n");
  float *b = NULL;
  printf("Initial address of b is %p\n", b);
  memory_allocator_2(b, n);
  // Uncomment to have segmentation fault due to a bad initialization:
  // in memory_allocator_2 we perform a local copy, the adress of "ary" doesn't exit the scope
  // print_array(b, n);
  // printf("Address of b after initialization is %p\n", b);

  /* --- THIRD EXAMPLE --- */
  /*
   * array of floats using a function that has as input
   * only the dimension and returns a pointer
   * 
   */
  printf("\n ====== 3rd EXAMPLE ====== \n");
  float *c;
  printf("Initial address of c is %p\n", c);
  memory_allocator_3(&c, n);
  print_array(a, n);
  printf("Address of c after initialization is %p\n", c);

  printf("\nFreeing the memory!\n");
  free(a);
  free(b);
  free(c);

  return 0;
}

/*
void print_array(float* ary, int dim){
  printf("[");
  for(int i = 0; i < dim; i++)
    printf(" %f ", ary[i]);
  printf("]\n");
}

float* memory_allocator_1(const int dim){
  float *ary = malloc(dim*sizeof(float));
  printf("memory_allocator_1 (%d elements) at address %p \n", dim, ary);
  for(int i = 0; i < dim; i++)
    ary[i] = i;
  return ary;
}

void memory_allocator_2(float* ary, int dim){
  ary = malloc(dim*sizeof(float));
  printf("memory_allocator_2 (%d elements) at address %p \n", dim, ary);
  for(int i = 0; i < dim; i++)
    ary[i] = i;
  printf("Display inside memory_allocator_2\n");
  print_array(ary, dim);
  printf("Exiting memory_allocator_2\n");
}

void memory_allocator_3(float** ary, int dim){

  *ary = malloc(dim*sizeof(float));
  for(int i = 0;  i < dim; i++)
    (*ary)[i] = i;
  printf("memory_allocator_3 (%d elements) at address %p, working on %p \n", dim, ary, *ary);
}*/