#include <sprites.h>

float sky_scale_x, sky_scale_y;

float get_sky_scale_x(void) {
    return sky_scale_x;
}

float get_sky_scale_y(void) {
    return sky_scale_y;
}

void set_sky_scale(float x, float y) {
    sky_scale_x = x;
    sky_scale_y = y;
}