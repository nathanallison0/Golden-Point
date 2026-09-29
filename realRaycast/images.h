#ifndef _GPOINT_IMAGES_H_
#define _GPOINT_IMAGES_H_

#include "../../SDL/start/include/SDL3Start.h"

#define get_graphics(g, x, y) g.pixels[((y) * g.width) + (x)]
#define get_graphics_p(g, x, y) g->pixels[((y) * g->width) + (x)]

typedef struct {
    Uint16 width, height;
    rgba *pixels;
} image;

extern image images[];

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

#endif