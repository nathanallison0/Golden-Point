#include "../../SDL/SDL3Start.h"

typedef struct {
    Uint32 width, height;
    float world_height_percent;
    rgba *pixels;
} array_sprite;

float sky_scale_x, sky_scale_y;

#define get_array_sprite(sprite, x, y) sprite.pixels[((y) * sprite.width) + (x)]

#include "spritesRaw.h"