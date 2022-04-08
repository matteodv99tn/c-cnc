#include "point.h"

#include <math.h>

//   __  __                          
//  |  \/  | __ _  ___ _ __ ___  ___ 
//  | |\/| |/ _` |/ __| '__/ _ \/ __|
//  | |  | | (_| | (__| | | (_) \__ \
//  |_|  |_|\__,_|\___|_|  \___/|___/
//

#define POINT_INSPECTION_DATA_BUFFER_SIZE 10
                                

//   ____       _   _                
//  / ___|  ___| |_| |_ ___ _ __ ___ 
//  \___ \ / _ \ __| __/ _ \ '__/ __|
//   ___) |  __/ |_| ||  __/ |  \__ \
//  |____/ \___|\__|\__\___|_|  |___/
//                   

void point_x(point_t *p, data_t value){

  p->x = value;
  p->s |= 1 << 1;

}

void point_y(point_t *p, data_t value){

  p->y = value;
  p->s |= 1 << 2;

}

void point_z(point_t *p, data_t value){

  p->z = value;
  p->s |= 1 << 3;

}

void point_xyz(point_t *p, data_t x_val, data_t y_val, data_t z_val){

  point_x(p, x_val);
  point_y(p, y_val);
  point_z(p, z_val);

}

//      _    _            _                  __                      
//     / \  | | __ _  ___| |__  _ __ __ _   / _|_   _ _ __   ___ ___ 
//    / _ \ | |/ _` |/ _ \ '_ \| '__/ _` | | |_| | | | '_ \ / __/ __|
//   / ___ \| | (_| |  __/ |_) | | | (_| | |  _| |_| | | | | (__\__ \
//  /_/   \_\_|\__, |\___|_.__/|_|  \__,_| |_|  \__,_|_| |_|\___|___/
//             |___/                                                 
//

data_t point_dist(point_t *from, point_t *to){

  data_t dx = to->x - from->x;
  data_t dy = to->y - from->y;
  data_t dz = to->z - from->z;

  return sqrt( pow(dx,2) + pow(dy, 2) + pow(dz, 2) );

}

void point_delta(point_t *from, point_t *to, point_t *delta){

  set_x(delta, to->x - from->x);
  set_x(delta, to->y - from->y);
  set_x(delta, to->z - from->z);

}

//    ___  _   _                   
//   / _ \| |_| |__   ___ _ __ ___ 
//  | | | | __| '_ \ / _ \ '__/ __|
//  | |_| | |_| | | |  __/ |  \__ \
//   \___/ \__|_| |_|\___|_|  |___/
//

void point_inspect(point_t *p, char **desc){


  
}

void point_modal(point_t *curr, point_t *prev){

  if( (curr->s & (1 << 1)) == 0 ) // bitmasking first bit of s
    point_x(curr, prev->x);
    
  if( (curr->s & (1 << 2)) == 0 ) // bitmasking second bit of s
    point_x(curr, prev->y);
    
  if( (curr->s & (1 << 3)) == 0 ) // bitmasking third bit of s
    point_x(curr, prev->z);

}







