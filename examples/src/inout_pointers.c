#include <stdio.h>
#include <stdlib.h>

#include "inout_pointers.h"

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
}