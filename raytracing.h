#ifndef __RAYTRACING_H
#define __RAYTRACING_H

#define UPPER_LIMIT 6000
#include "objects.h"
#include <stdint.h>

//TODO
/*You can modify raytracing prototype here
* or create your new header*/

void raytracing(uint8_t *pixels, color background_color,
                rectangular_node rectangulars, sphere_node spheres,
                light_node lights, const viewpoint *view,
                int width, int height);
#endif
