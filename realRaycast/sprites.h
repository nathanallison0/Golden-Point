#include "../../SDL/start/include/SDL3Start.h"

#define NUM_ROT_SPRITE_FRAMES 8
#define ROT_SPRITE_INCR ((M_PI * 2) / NUM_ROT_SPRITE_FRAMES)

typedef struct {
    Uint16 width, height;
    float world_height_percent;
    float origin_y_offset_percent;
    Uint8 is_rot;
    rgba *pixels;
} sprite;

float sky_scale_x, sky_scale_y;