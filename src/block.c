#include <ctype.h>

#include "block.h"
//   ____       _            _
//  |  _ \ _ __(_)_   ____ _| |_ ___
//  | |_) | '__| \ \ / / _` | __/ _ \
//  |  __/| |  | |\ V / (_| | ||  __/
//  |_|   |_|  |_| \_/ \__,_|\__\___|
// Static, or private, functions
// are only accessible from within this file

#define LAMBDA_SIZE 1.0E6
#define is_arc(b) (b->type == ARC_CCW || b->type == ARC_CW)

// quantize a time interval as a multiple of the sampling time
// tq; put the difference in dq
static data_t quantize(data_t t, data_t tq, data_t *dq) {
  data_t q;
  q = ((index_t)(t / tq) + 1) * tq;
  *dq = q - t;
  return q;
}

// set individual fields for a G-code word, made by a command
// (single letter) and an argument (number as a string)
static int block_set_field(block_t *b, char cmd, char *arg) {
  assert(b);
  switch (cmd) {
  case 'N':
    b->n = atol(arg);
    break;
  case 'G':
    b->type = atoi(arg);
    break;
  case 'X':
    point_x(&b->target, atof(arg));
    break;
  case 'Y':
    point_y(&b->target, atof(arg));
    break;
  case 'Z':
    point_z(&b->target, atof(arg));
    point_z(&b->center, atof(arg));
    break;
  case 'F':
    b->feedrate = atof(arg);
    break;
  case 'S':
    b->spindle = atof(arg);
    break;
  case 'T':
    b->tool = atoi(arg);
    break;
  case 'I':
    point_x(&b->center, atof(arg));
    b->radius = 0; // reset radius
    break;
  case 'J':
    point_y(&b->center, atof(arg));
    b->radius = 0; // reset radius
    break;
  case 'R':
    b->radius = atof(arg);
    point_x(&b->center, 0); // reset center
    point_y(&b->center, 0); // reset center
    break;
  default:
    fprintf(stderr, "ERROR: Unexpected G-code command %c%s\n", cmd, arg);
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}

// compute arc: if R is not zero, calculate center
//              if R is zero, calculate it from center
static void block_compute_arc(block_t *b) {
  data_t af, at; // angles
  data_t xt = b->target.x;
  data_t xf = b->prev->target.x;
  data_t yt = b->target.y;
  data_t yf = b->prev->target.y;
  // calculate radius from center
  if (b->radius == 0) {
    b->radius = sqrt(pow(b->center.x, 2) + pow(b->center.y, 2));
  }
  // calculate center from radius and extreme points
  else {
    int cw = b->type == ARC_CW ? 1 : 0;
    data_t d = sqrt(pow(xf-xt, 2) + pow(yf-yt, 2));
    data_t l = d/2.0;
    data_t h = sqrt(pow(b->radius,2)-pow(l,2));
    int s = b->radius/fabs(b->radius) * (cw ? 1 : -1);
    data_t x = l/d*(xt-xf)+s*h/d*(yt-yf)+xf;
    data_t y = l/d*(yt-yf)-s*h/d*(xt-xf)+yf;
    point_x(&b->center, x - b->prev->target.x);
    point_y(&b->center, y - b->prev->target.y);
  }
  // calculate initial and final angles
  af = atan2(-b->center.y, -b->center.x);
  if (af < 0) af += 2*M_PI;
  at = atan2(b->target.y - b->prev->target.y - b->center.y, 
             b->target.x - b->prev->target.x - b->center.x);
  if (at < 0) at += 2*M_PI;
  if (at == 0) at = 2*M_PI;
  // use b->delta for storing initial angle, final angle, and arc
  point_x(&b->delta, af);
  point_y(&b->delta, at);
  if (b->type == ARC_CCW) {
    if ( b->radius > 0)
      point_z(&b->delta, fabs(at - af));
    else
      point_z(&b->delta, 2*M_PI - fabs(at - af));
  }
  else {
    if ( b->radius > 0)
      point_z(&b->delta, -(2*M_PI - fabs(at - af)));
    else
      point_z(&b->delta, -fabs(at - af)); 
  }
  // calculate arc length
  b->length = fabs(b->delta.z * b->radius);
}

// compute velocity profile for the block
void block_compute_profile(block_t *b) {
  assert(b);
  data_t A, D, a, d;
  data_t dt, dt_1, dt_2, dt_m, dq;
  data_t f_n, f_m, l;
  data_t f_s, f_e;

  A = b->config->A;
  D = b->config->D;
  f_n = b->feedrate / 60.0;
  f_m = b->feedrate_max / 60;
  f_s = b->feedrate_in / 60.0;
  f_e = b->feedrate_out / 60.0;
  l = b->length;

  a = A, d = -D;
  
  // Some of the following conditionals could be condensed.
  // They are INTENTIONALLY kept separate in order to enhance readability.

  // initial and final speed less than feedrate: trapezoid or triangle
  if (f_m > f_s && f_m > f_e) {
    dt_1 = fabs(f_m - f_s) / A;
    dt_2 = fabs(f_m - f_e) / D;
    if (f_m < f_n) { // can't reach feedrate: triangle
      dt_m = 0;
    }
    else { // trapezoid
      dt_m = l / f_m - (dt_1 * (f_m + f_s) + dt_2 * (f_e + f_m)) / (2.0 * f_m);
    }
    // final speed zero:
    // reshape TRAPEZOIDAL profile to end on tq multiple
    if (f_e <= 0 && dt_m > 0) {
      dt = quantize(dt_1 + dt_m + dt_2, b->config->tq, &dq);
      dt_m -= dq;
      dt_2 += 2*dq;
      d = - f_m / dt_2;
    }
    // reshape TRIANGULAR profile to end on tq multiple
    else if (f_e <= 0 && dt_m <= 0) {
      dt = quantize(dt_1 + dt_2, b->config->tq, &dq);
      dt_2 += dq;
      f_m = -((dt_2*f_e + dt_1*f_s - 2*l)/(dt_1 + dt_2));
      a = f_m / dt_1;
      d = -f_m / dt_2;
    }
    else {
      dt = dt_1 + dt_2 + dt_m;
    }
  }
  // initial speed equal to feedrate
  else if (f_m <= f_s && f_m > f_e) {
    dt_1 = 0;
    dt_2 = fabs(f_m - f_e) / D;
    dt_m = l / f_m - dt_2 * (f_m+f_e)/(2*f_m);
    // final speed zero: reshape profile to end on tq multiple
    if (f_e <= 0) {
      dt = quantize(dt_m + dt_2, b->config->tq, &dq);
      dt_m -= dq;
      dt_2 += 2*dq;
      d = - f_m / dt_2;
    }
    else {
      dt = dt_2 + dt_m;
    }
  }
  // final speed equal to feedrate
  else if (f_m > f_s && f_m <= f_e) {
    dt_1 = fabs(f_m - f_s) / A;
    dt_2 = 0;
    dt_m = l / f_m - dt_1 * (f_m+f_s)/(2*f_m);
    dt = dt_1 + dt_m;
  }
  // constant speed
  else {
    dt_1 = 0;
    dt_2 = 0;
    dt_m = l / f_m;
    dt = dt_m;
  }
  
  // set back  values into profile structure:
  b->prof->dt_1 = dt_1;
  b->prof->dt_m = dt_m;
  b->prof->dt_2 = dt_2;
  b->prof->a = a;
  b->prof->d = d;
  b->prof->f = f_m;
  b->prof->fs = f_s;
  b->prof->fe = f_e;
  b->prof->dt = (b->type == RAPID ? 60 : dt);
  b->prof->l = l;
}

// returns a valid point for the previous block: origin if the previous
// block is undefined
static point_t point_zero(block_t *b) {
  point_t p0;
  if (b->prev == NULL) {
    p0 = point_new();
    point_xyz(&p0, 0, 0, 0);
  }
  else {
    p0 = b->prev->target;
  }
  return p0;
}

static data_t blocks_angle(block_t *b0, block_t *b1) {
  data_t angle = M_PI;
  point_t center = point_new();
  if (b0 == NULL || 
    b0->prev == NULL ||
    !point_allset(&b0->target) || 
    !point_allset(&b0->prev->target) ||
    !point_allset(&b1->target)) 
    return angle;
  if (b0->length == 0 || b1->length == 0)
    return angle;
  // Calculate direction vectors
  if (!is_arc(b0) && !is_arc(b1)) { // line-line
    angle = M_PI - point_angle(&b0->prev->target, &b0->target, &b1->target);
  }
  else if (is_arc(b0) && !is_arc(b1)) { // arc-line
    point_xyz(&center, b0->prev->target.x + b0->center.x, b0->prev->target.y + b0->center.y, b0->center.z);
    angle = M_PI_2 - point_angle(&center, &b0->target, &b1->target);
  }
  else if (!is_arc(b0) && is_arc(b1)) { // line-arc
    point_xyz(&center, b0->target.x + b1->center.x, b0->target.y + b1->center.y, b1->center.z);
    angle = M_PI_2 - point_angle(&b0->prev->target, &b0->target, &center);
  }
  else { // arc-arc
    point_t center0 = point_new();
    point_xyz(&center, b0->target.x + b1->center.x, b0->target.y + b1->center.y, b0->center.z);
    point_xyz(&center0, b0->prev->target.x + b0->center.x, b0->prev->target.y + b0->center.y, b0->center.z);
    angle = M_PI - point_angle(&center0, &b0->target, &center);
  }
  return angle;
}

//   ____        _     _ _
//  |  _ \ _   _| |__ | (_) ___
//  | |_) | | | | '_ \| | |/ __|
//  |  __/| |_| | |_) | | | (__
//  |_|    \__,_|_.__/|_|_|\___|
// functions

block_t *block_new(char *line, block_t *prev, struct machine_config *cfg) {
  assert(line);
  block_t *b = malloc(sizeof(block_t));
  assert(b);

  // memory setup
  if (prev) { // if there is a previous block
    memcpy(b, prev, sizeof(block_t));
    b->prev = prev;
    prev->next = b;
  }
  else { // this is the first block
    memset(b, 0, sizeof(block_t));
    b->prev = NULL; // redundant
    b->type = NO_MOTION;
  }

  // fields that must be calculated
  b->length = 0.0;
  b->target = point_new();
  b->delta = point_new();
  b->feedrate_in = 0.0;
  b->feedrate_out = 0.0;
  // allocate memory for referenced objects:
  b->prof = malloc(sizeof(block_profile_t));
  assert(b->prof);
  // copy line to b->line
  b->line = malloc(strlen(line) + 1);
  strcpy(b->line, line);
  b->config = cfg;
  // b->type = NO_MOTION;
  return b;
}

void block_free(block_t *block) {
  assert(block);
  // only block line and prof if they are not NULL
  if (block->line)
    free(block->line);
  if (block->prof)
    free(block->prof);
  free(block);
}

int block_parse(block_t *block) {
  assert(block);
  char *line, *word, *to_free, lone;
  point_t p0;
  int rv = EXIT_SUCCESS;

  // strsep changes the string on which it operates
  // so we make a copy of it. Also, keep track of the original copy
  // pointer so that at the end we can free it (to_free)
  to_free = line = strdup(block->line); // uses malloc internally

  // loop and split line into words
  while ((word = strsep(&line, " ")) != NULL) {
    if (word[0] == '#') // comment
      break; 
    if (strlen(word) == 0) // multiple spaces
      continue;
    if (strlen(word) == 1 && lone == '\0') { // lone command
      lone = word[0];
      continue;
    }
    // parse each word
    // "x123.9" -> 'X', "123.9"
    if (lone != 0) {
      rv = block_set_field(block, toupper(lone), word);
      lone = '\0';
    }
    else
      rv = block_set_field(block, toupper(word[0]), word + 1);
  }
  free(to_free);

  // inherit from previous block
  p0 = point_zero(block);
  point_modal(&p0, &block->target);

  // compute the fields to be calculated
  switch (block->type) {
  case RAPID:
  case LINE:
    point_delta(&p0, &block->target, &block->delta);
    block->length = point_dist(&p0, &block->target);
    break;
  case ARC_CW:
  case ARC_CCW:
    block_compute_arc(block);
    break;
  default:
    break;
  }

  block->angle = fabs(blocks_angle(block->prev, block));
  block_compute_profile(block);

  return rv;
}

// Calculate lambda value at a given time
data_t block_lambda(block_t *b, data_t t) {
  assert(b);
  data_t dt_1 = b->prof->dt_1;
  data_t dt_2 = b->prof->dt_2;
  data_t dt_m = b->prof->dt_m;
  data_t a = b->prof->a;
  data_t d = b->prof->d;
  data_t f = b->prof->f;
  data_t fs = b->prof->fs;
  data_t fe = b->prof->fe;
  data_t r;

  if (b->n == 200) { 
    int a = 1;
  }
  if (t < 0) { // negative time
    r = 0.0;
  }
  else if (t < dt_1) { // acceleration
    r = a * pow(t, 2) / 2.0 + fs * t;
    r /= b->prof->l;
  }
  else if (t < (dt_1 + dt_m)) { // maintenance
    t -= dt_1;
    r = a * pow(dt_1, 2) / 2 + fs * dt_1 + f * t;
    r /= b->prof->l;
  }
  else if (t < (dt_1 + dt_m + dt_2)) { // deceleration
    t -= dt_1 + dt_m;
    r = a * pow(dt_1, 2) / 2 + fs * dt_1 + f * dt_m;
    r += (f * t + d * pow(t, 2) / 2.0);
    r /= b->prof->l;
  }
  // else if (t < dt_1) { // acceleration
  //   r = a * pow(t, 2) / 2.0;
  //   r /= b->prof->l;
  // }
  // else if (t < (dt_1 + dt_m)) { // maintenance
  //   r = f * (dt_1 / 2.0 + (t - dt_1));
  //   r /= b->prof->l;
  // }
  // else if (t < (dt_1 + dt_m + dt_2)) { // deceleration
  //   data_t t_2 = dt_1 + dt_m;
  //   r = f * dt_1 / 2.0 + f * (dt_m + t - t_2) +
  //       d / 2.0 * (pow(t, 2) + pow(t_2, 2)) - d * t * t_2;
  //   r /= b->prof->l;
  // }
  else { // after ending time
    r = 1.0;
  }
  
  // round result to avoid rounding errors in comparison
  // failing to do so would result in values slightly larger than 1
  return round(r * LAMBDA_SIZE) / LAMBDA_SIZE;
}

// interpolate axes positions at a given lambda value:
// x(t) = x_0 + Dx * lambda(t)
point_t block_interpolate(block_t *b, data_t lambda) {
  assert(b);
  point_t result = point_new();
  point_t p0 = point_zero(b);
  data_t da;
  switch (b->type)
  {
  case LINE:
    point_x(&result, p0.x + b->delta.x * lambda);
    point_y(&result, p0.y + b->delta.y * lambda);
    point_z(&result, p0.z + b->delta.z * lambda);
    break;

  case ARC_CCW:
  case ARC_CW:
    da = b->delta.z;
    point_x(&result, p0.x + b->center.x + fabs(b->radius)*cos(b->delta.x + da * lambda));
    point_y(&result, p0.y + b->center.y + fabs(b->radius)*sin(b->delta.x + da * lambda));
    point_z(&result, p0.z);
    break;
  
  default:
    break;
  }

  return result;
}

// print block description
void block_print(block_t *b, FILE *out) {
  assert(b);
  assert(out);
  point_t p0 = point_zero(b);
  char *t, *p;

  point_inspect(&b->target, &t);
  point_inspect(&p0, &p);

  fprintf(out, "%03u: %s -> %s F%7.1f,%7.1f,%7.1f S%7.1f T%02u (%d)\n", 
          b->n, p, t, b->feedrate_in,
          b->feedrate, b->feedrate_out, b->spindle, b->tool, b->type);
  fprintf(out, "  IJR[%8.3f %8.3f %8.3f] L%8.3f Delta[%8.3f %8.3f %8.3f] %6.3f\n", b->center.x, b->center.y, b->radius, b->length, b->delta.x, b->delta.y, b->delta.z, b->angle / M_PI * 180);

  // CRUCIAL!!! or you'll have memory leaks!
  free(t);
  free(p);
}

#ifdef BLOCK_MAIN
int main() {
  block_t *b1, *b2;
  data_t t, lambda;
  struct machine_config cfg = {.A = 10, .D = 5, .tq = 0.005};
  point_t p = point_new();
  char *p_desc;

  b1 = block_new("N10 G00 X100 Y100 z100 t3", NULL, &cfg);
  block_parse(b1);
  b2 = block_new("N20 G01 Y120 X110 f1000 s2000", b1, &cfg);
  block_parse(b2);

  block_print(b1, stderr);
  block_print(b2, stderr);

  for (t = 0; t <= b2->prof->dt; t += cfg.tq) {
    lambda = block_lambda(b2, t);
    p = block_interpolate(b2, lambda);
    printf("%f %f %f %f %f\n", t, lambda, p.x, p.y, p.z);
  }

  block_free(b1);
  block_free(b2);
}
#endif
