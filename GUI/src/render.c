#include "../include/elements.h"
#include "../include/render.h"
#include "../../BasicFont/BasicFont.h"

void render_notype(GUI_Element *notype_element, int x, int y) {

}

alstring *write_text_word_wrap(char *text, int font_size, int wrap_length, int *num_lines) {
    alstring *rendered = alstring_init(10);

    int wrap_length_chars = wrap_length / (font_size * (BF_CHAR_WIDTH + 1));
    int line_count = 1;

    // If there isn't room to draw a single char, draw as one column
    if (wrap_length_chars <= 2) {
        size_t text_length = strlen(text);
        for (size_t i = 0; i < text_length; i++) {
            if (text[i] != ' ' && text[i] != '\n') {
                alstring_append(rendered, text[i]);
                alstring_append(rendered, '\n');
                line_count++;
            }
        }
        if (num_lines) {
            *num_lines = line_count;
        }
        return rendered;
    }

    int write_col = 0;
    bool drawing = true;
    while (drawing) {
        char *sep_addr = strchr(text, ' ');
        char *newline_addr = strchr(text, '\n');

        // If there is a newline before the space, separate by newline
        if (newline_addr && newline_addr < sep_addr) {
            sep_addr = newline_addr;
        }

        // If there are no spaces left, draw until end of text
        if (!sep_addr) {
            sep_addr = strchr(text, '\0');
            drawing = false;
        }

        int word_char_count = sep_addr - text;

        // If word is longer than wrap length, wrap by char
        if (word_char_count >= wrap_length_chars) {
            // Draw up until wrap x
            text += alstring_appendsub(rendered, text, wrap_length_chars - write_col);
            alstring_append(rendered, '\n'); line_count++;

            // Draw full rows of text until rest of word will fit in a row
            while (sep_addr - text > wrap_length_chars) {
                text += alstring_appendsub(rendered, text, wrap_length_chars);
                alstring_append(rendered, '\n'); line_count++;
            }

            // Draw remaining chars
            write_col = alstring_appendsub(rendered, text, sep_addr - text);
            text += write_col;
        } else {
            // The word fits within wrap length
            // If drawing will end us past wrap length (word won't fit), newline
            if (write_col + word_char_count > wrap_length_chars) {
                alstring_append(rendered, '\n'); line_count++;
                write_col = 0;
            }

            // Draw word
            int written = alstring_appendsub(rendered, text, sep_addr - text);
            write_col += written;
            text += written;
        }
        
        // Add separator
        if (sep_addr == newline_addr) {
            alstring_append(rendered, '\n'); line_count++;
            write_col = 0;
        } else {
            alstring_append(rendered, ' ');
            write_col++;
        }
        text++;
    }

    if (num_lines) {
        *num_lines = line_count;
    }
    return rendered;
}

void render_text(GUI_Element *text_element, int x, int y) {
    GUI_EText *subdata = (GUI_EText *) text_element->subdata;

    Uint16 draw_width = text_element->width - text_element->padding_left - text_element->padding_right;
    Uint16 draw_height = text_element->height - text_element->padding_bottom - text_element->padding_top;

    int formatted_lines;
    alstring *formatted = write_text_word_wrap(subdata->text, subdata->font_size, draw_width, &formatted_lines);

    // Get y offset based off center_y attribute
    int draw_y_offset = 0;
    if (text_element->center_y != GUI_CENTERY_TOP) {
        int text_height = subdata->font_size * ((formatted_lines * (BF_CHAR_HEIGHT + 1)) - 1);
        draw_y_offset = text_element->center_y == GUI_CENTERY_MIDDLE ? (draw_height - text_height) / 2 : draw_height - text_height;
    }

    // Draw line by line
    char *start_pos = formatted->text;
    for (int line = 0; line < formatted_lines; line++) {
        char *end_pos = strchr(start_pos, line == formatted_lines - 1 ? '\0' : '\n');
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
    }

    alstring_destroy(formatted);
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
                node->element->border_width * 2 +
                node->element->width +
                node->margin_front;
        }
    } else {
        render_coord = y;
        for (GUI_EContainerNode *node = subdata->head; node; node = node->next) {
            GUI_RenderElement(node->element, x, render_coord + node->margin_back);
            render_coord +=
                node->margin_back +
                node->element->border_width * 2 +
                node->element->height +
                node->margin_front;
        }
    }
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