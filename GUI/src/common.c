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

Uint16 get_container_elements_width(GUI_Element *container) {
    GUI_EContainer *subdata = container->subdata;

    Uint16 total_width = 0;
    for (GUI_EContainerNode *node = subdata->head; node; node = node->next) {
        total_width += node->margin_back + get_total_width(node->element) + node->margin_front;
    }
    return total_width;
}

Uint16 get_container_elements_height(GUI_Element *container) {
    GUI_EContainer *subdata = container->subdata;

    Uint16 total_height = 0;
    for (GUI_EContainerNode *node = subdata->head; node; node = node->next) {
        total_height += node->margin_back + get_total_height(node->element) + node->margin_front;
    }
    return total_height;
}