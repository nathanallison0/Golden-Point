#include "../include/common.h"

Uint16 get_draw_width(GUI_Element *element) {
    return element->width - element->padding_left - element->padding_right;
}

Uint16 get_draw_height(GUI_Element *element) {
    return element->height - element->padding_bottom - element->padding_top;
}

void set_draw_width(GUI_Element *element, Uint16 draw_width) {
    element->width = element->padding_right + draw_width + element->padding_left;
}

void set_draw_height(GUI_Element *element, Uint16 draw_height) {
    element->height = element->padding_top + draw_height + element->padding_bottom;
}

Uint16 get_total_width(GUI_Element *element) {
    return element->width + element->border_width * 2;
}

Uint16 get_total_height(GUI_Element *element) {
    return element->height + element->border_width * 2;
}