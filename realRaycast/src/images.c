#include <images.h>
#include <imagesRaw.h>
#include <constants.h>

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