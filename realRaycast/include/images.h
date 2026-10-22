#ifndef _GPOINT_IMAGES_H_
#define _GPOINT_IMAGES_H_

#include <SDL3Start.h>

#define get_graphics(g, x, y) g.pixels[((y) * g.width) + (x)]
#define get_graphics_p(g, x, y) g->pixels[((y) * g->width) + (x)]

typedef struct {
    Uint16 width, height;
    rgba *pixels;
} image;

#include <imagesRaw.h>

#define SKY_IMAGE images[image_sky]

/**
 * Draw an image.
 * 
 * \param index The index of the image
 * \param x The x-value to draw from
 * \param y The y-value to draw from
 */
void draw_image(Uint16 index, int x, int y);

/**
 * Draw an image at a scale.
 * 
 * \param index The index of the image
 * \param x The x-value to draw from
 * \param y The y-value to draw from
 * \param scale The scale to draw at
 */
void draw_image_scale(Uint16 index, int x, int y, int scale);

#endif