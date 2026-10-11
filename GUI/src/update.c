#include "../include/update.h"
#include "../include/common.h"

void update_text_data(GUI_Element *text_element) {
    GUI_EText *subdata = text_element->subdata;

    if (subdata->formatted_text) {
        alstring_destroy(subdata->formatted_text);
    }

    // If the width is auto, set the width based off longest line of text
    if (text_element->width_auto) {
        // Find max line length in text
        int max_line_length = 0;
        char *start_addr = subdata->text;
        bool loop = true;
        while (loop) {
            char *newline_addr = strchr(start_addr, '\n');
            if (!newline_addr) {
                newline_addr = strchr(start_addr, '\0');
                loop = false;
            }

            int this_line_length = newline_addr - start_addr;
            if (this_line_length > max_line_length) {
                max_line_length = this_line_length;
            }

            start_addr = newline_addr + 1;
        }

        set_draw_width(text_element, subdata->font_size * (max_line_length * (BF_CHAR_WIDTH + 1) - 1));
    }

    // Format text based off width
    Uint16 draw_width = get_draw_width(text_element);
    subdata->formatted_text = BF_WriteTextWordWrap(subdata->text, subdata->font_size, draw_width, &subdata->num_lines);

    // If the height is auto, set the height based off number of lines of formatted text
    if (text_element->height_auto) {
        set_draw_height(text_element, subdata->font_size * (subdata->num_lines * (BF_CHAR_HEIGHT + 1) - 1));
    }
}

void update_container_data(GUI_Element *container) {
    GUI_EContainer *subdata = container->subdata;

    if (container->width_auto) {
        if (subdata->num_elements == 0) {
            set_draw_width(container, 0);
        } else if (subdata->is_horizontal) {
            // Set the width to the sum of all element widths
            set_draw_width(container, get_container_elements_width(container));
        } else {
            // Set the width to be the max width of all elements
            Uint16 max_width = get_total_width(subdata->head->element);
            for (GUI_EContainerNode *node = subdata->head->next; node; node = node->next) {
                Uint16 width = get_total_width(node->element);
                if (width > max_width) {
                    max_width = width;
                }
            }

            set_draw_width(container, max_width);
        }
    }

    if (container->height_auto) {
        if (subdata->num_elements == 0) {
            set_draw_height(container, 0);
        } else if (subdata->is_horizontal) {
            // Get the height to be the max height of all elements
            Uint16 max_height = get_total_height(subdata->head->element);
            for (GUI_EContainerNode *node = subdata->head->next; node; node = node->next) {
                Uint16 height = get_total_height(node->element);
                if (height > max_height) {
                    max_height = height;
                }
            }

            set_draw_height(container, max_height);
        } else {
            // Get the total height of all elements
            set_draw_height(container, get_container_elements_height(container));
        }
    }
}

void (*update_functions[GUI_NUM_ETYPES])(GUI_Element *) = {
    NULL, // notype
    update_text_data,
    update_container_data
};

void GUI_UpdateElementStatic(GUI_Element *element) {
    if (update_functions[element->type]) {
        update_functions[element->type](element);
    }
}

void GUI_UpdateElement(GUI_Element *element) {
    GUI_UpdateElementStatic(element);
    // TODO: update container
}