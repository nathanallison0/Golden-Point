#include "../../SDL/SDL3Start.h"

#define get_graphics(g, x, y) g.pixels[((y) * g.width) + (x)]
#define get_graphics_p(g, x, y) g->pixels[((y) * g->width) + (x)]

// Images
typedef struct {
    Uint16 width, height;
    rgba *pixels;
} image;

#include "imagesRaw.h"

void draw_image(Uint16 index, int x, int y) {
    int end_x = min(x + images[index].width, WINDOW_WIDTH);
    int end_y = min(y + images[index].height, WINDOW_HEIGHT);
    Uint16 image_x = 0;
    Uint16 image_y = 0;
    for (int row = y; row < end_y; row++) {
        for (int col = x; col < end_x; col++) {
            set_pixel_rgba(col, row, get_graphics(images[index], image_x, image_y));
            image_x++;
        }
        image_x = 0;
        image_y++;
    }
}

void draw_image_scale(Uint16 index, int x, int y, int scale) {
    int end_x = min(x + (images[index].width * scale), WINDOW_WIDTH);
    int end_y = min(y + (images[index].height * scale), WINDOW_HEIGHT);
    Uint16 image_x = 0;
    Uint16 image_y = 0;
    for (int row = y; row < end_y; row += scale) {
        for (int col = x; col < end_x; col += scale) {
            draw_rect_rgba(col, row, scale, scale, get_graphics(images[index], image_x, image_y));
            image_x++;
        }
        image_x = 0;
        image_y++;
    }
}

#define SKY_IMAGE images[image_sky]

// Sprites
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

#include "spritesRaw.h"