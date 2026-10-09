#include "../include/elements.h"
#include "../include/render.h"
#include "../include/common.h"
#include "../../BasicFont/include/BasicFont.h"

void render_notype(GUI_Element *notype_element, int x, int y) {

}

void render_text(GUI_Element *text_element, int x, int y) {
    GUI_EText *subdata = (GUI_EText *) text_element->subdata;

    Uint16 draw_width = get_draw_width(text_element);
    Uint16 draw_height = get_draw_height(text_element);

    // Get y offset based off center_y attribute
    int draw_y_offset = 0;
    if (text_element->center_y != GUI_CENTERY_TOP) {
        int text_height = subdata->font_size * ((subdata->num_lines * (BF_CHAR_HEIGHT + 1)) - 1);
        draw_y_offset = text_element->center_y == GUI_CENTERY_MIDDLE ? (draw_height - text_height) / 2 : draw_height - text_height;
    }

    // Draw line by line
    char *start_pos = subdata->formatted_text->text;
    for (int line = 0; line < subdata->num_lines; line++) {
        char *end_pos = strchr(start_pos, line == subdata->num_lines - 1 ? '\0' : '\n');
        char original_end = *end_pos;
        *end_pos = '\0';

        // Offset draw x based off center x
        int draw_x_offset = 0;
        if (text_element->center_x != GUI_CENTERX_LEFT) {
            // Don't count spaces in line width
            int num_spaces = 0;
            while (*(end_pos - num_spaces - 1) == ' ') {
                num_spaces++;
            }

            int line_width = subdata->font_size * ((strlen(start_pos) - num_spaces) * (BF_CHAR_WIDTH + 1) - 1);
            draw_x_offset = text_element->center_x == GUI_CENTERX_MIDDLE ? (draw_width - line_width) / 2 : draw_width - line_width;            
        }

        BF_DrawText(start_pos, x + draw_x_offset, y + draw_y_offset + line * subdata->font_size * (BF_CHAR_HEIGHT + 1), subdata->font_size, -1, subdata->font_color, false);

        start_pos = end_pos + 1;
        *end_pos = original_end;
    }
}

void render_container(GUI_Element *container_element, int x, int y) {
    GUI_EContainer *subdata = container_element->subdata;
    int render_coord;
    if (subdata->is_horizontal) {
        render_coord = x;
        for (GUI_EContainerNode *node = subdata->head; node; node = node->next) {
            GUI_RenderElement(node->element, render_coord + node->margin_back, y);
            render_coord += 
                node->margin_back +
                get_total_width(node->element) +
                node->margin_front;
        }
    } else {
        render_coord = y;
        for (GUI_EContainerNode *node = subdata->head; node; node = node->next) {
            GUI_RenderElement(node->element, x, render_coord + node->margin_back);
            render_coord +=
                node->margin_back +
                get_total_height(node->element) +
                node->margin_front;
        }
    }
}

void (*render_functions[GUI_NUM_ETYPES])(GUI_Element *, int, int) = {
    render_notype,
    render_text,
    render_container
};

void GUI_RenderElement(GUI_Element *element, int x, int y) {
    Uint16 full_width = get_total_width(element);

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