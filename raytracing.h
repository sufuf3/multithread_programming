#ifndef __RAYTRACING_H
#define __RAYTRACING_H

#define UPPER_LIMIT 6000
#include "objects.h"
#include <stdint.h>

//TODO
/*You can modify raytracing prototype here
* or create your new header*/

typedef struct rayarg {
    uint8_t *pixels;
    double *background_color;
    rectangular_node rectangulars;
    sphere_node spheres;
    light_node lights;
    viewpoint *view;
    int  width;
    int height;
    int thread_num;
    int core_num;
} rayargs;

//void raytracing(uint8_t *pixels, color background_color,
rayargs *ray(uint8_t *pixels, color background_color,
             rectangular_node rectangulars, sphere_node spheres,
             light_node lights, viewpoint *view,
             int width, int height, int thread_num, int core_num);
void raytracing( void * r);
#endif
