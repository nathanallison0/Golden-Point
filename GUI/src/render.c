#include "../include/elements.h"
#include "../include/render.h"
#include "../../BasicFont/BasicFont.h"

void render_notype(GUI_Element *notype_element, int x, int y) {

}

void render_text(GUI_Element *text_element, int x, int y) {
    GUI_EText *subdata = (GUI_EText *) text_element->subdata;

    Uint16 draw_width = text_element->width - text_element->padding_right;
    Uint16 draw_height = text_element->height - text_element->padding_bottom;

    BF_DrawText(subdata->text, x, y, subdata->font_size, draw_width, subdata->font_color, false);
}

void render_container(GUI_Element *container_element, int x, int y) {

}

void (*render_functions[])(GUI_Element *, int, int) = {
    render_notype,
    render_text,
    render_container
};

void GUI_RenderElement(GUI_Element *element, int x, int y) {
    Uint16 full_width = element->width + element->border_width * 2;
    //Uint16 full_height = element->height + element->border_width * 2;

    // Render border
    draw_rect_rgba(x, y, full_width, element->border_width, element->border_color); // full top row
    draw_rect_rgba(x, y + element->border_width, element->border_width, element->height, element->border_color); // middle row left
    draw_rect_rgba(x + element->border_width + element->width, y + element->border_width, element->border_width, element->height, element->border_color); // middle row right
    draw_rect_rgba(x, y + element->height + element->border_width, full_width, element->border_width, element->border_color); // full bottom row

    // Render fill
    draw_rect_rgba(x + element->border_width, y + element->border_width, element->width, element->height, element->fill_color);

    // Render element contents
    render_functions[element->type](element, x + element->border_width + element->padding_left, y + element->border_width + element->padding_top);
}