#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include <math.h>

#include <gsl/gsl_blas.h>
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_vector.h>

typedef struct {
  double x;
  double y;
  double z;
} point_t;

void print_point(point_t* pt){
  printf("(%6.3f, %6.3f, %6.3f)", pt->x, pt->y, pt->z);
}


int main(){

  point_t pt    = { .x = 10, .y = 3, .z = 7 };  // Input point
  point_t d     = { .x = 10, .y = 0, .z = 5 };  // Translation value
  double  angle = 45.0f;                        // Rotation angle

  fprintf(stdout, "Starting point: ");
  print_point(&pt);
  printf("\n");
  fprintf(stdout, "Translating by: ");
  print_point(&d);
  printf("\n");
  fprintf(stdout, "Rotating by   : %6.3f degrees \n", angle);

  return 0;
}




