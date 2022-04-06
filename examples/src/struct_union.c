#include <stdio.h>
#include <stdlib.h>

typedef double data_t;

typedef struct {
  data_t x, y, z;
} point_t;

void point_print(point_t* a){
  printf("(%f, %f, %f)", a->x, a->y, a->z);
}

int main(){

  point_t pt;
  pt.x = 10;
  pt.y = 2;
  pt.z = 3;

  point_print(&pt); 


  return 0;
}

