//   ____       _       _   
//  |  _ \ ___ (_)_ __ | |_ 
//  | |_) / _ \| | '_ \| __|
//  |  __/ (_) | | | | | |_ 
//  |_|   \___/|_|_| |_|\__|
//                         

#ifndef _POINT_H_
#define _POINT_H_

#include "defines.h"

// Point's struct
typedef struct{

  data_t x;   // x coordinate
  data_t y;   // y coordinate
  data_t z;   // z coordinate

  // To determine if the values are set a bit-mask is used!
  // 0000 0000 => none set(0)
  // 0000 0001 => x is set(1)
  // 0000 0010 => y is set(2)
  // 0000 0100 => z is set(4)
  // 0000 0111 => xyz are al set (7)
  uint8_t s;

} point_t;

//   _____                 _   _                 
//  |  ___|   _ _ __   ___| |_(_) ___  _ __  ___ 
//  | |_ | | | | '_ \ / __| __| |/ _ \| '_ \/ __|
//  |  _|| |_| | | | | (__| |_| | (_) | | | \__ \
//  |_|   \__,_|_| |_|\___|\__|_|\___/|_| |_|___/
//

// Create a point
point_t point_new();

// Set coordinates
void point_x  (point_t *p, data_t value);
void point_y  (point_t *p, data_t value);
void point_z  (point_t *p, data_t value);
void point_xyz(point_t *p, data_t x_val, data_t y_val, data_t z_val);

// Distance between two points
data_t point_dist(point_t *from, point_t *to);

// Projections
void point_delta(point_t *from, point_t *to, point_t *delta);

// Inspection
void point_inspect(point_t *p, char **desc);

// "Modal behaviour": a point may have undefined coordinates
// and if so it must be able to inherit undefined coordinates
// from the previous point
void point_modal(point_t *curr, point_t *prev);

#endif // _POINT_H_