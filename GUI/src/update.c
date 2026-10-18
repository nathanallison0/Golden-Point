#include "../include/update.h"
#include "../include/common.h"
#include <stdlib.h>

void update_text_data(GUI_Element *text_element) {
    GUI_EText *subdata = text_element->subdata;

    if (subdata->formatted_text) {
        alstring_destroy(subdata->formatted_text);
    }

    // If the width is auto, set the width based off longest line of text
    if (text_element->width_dim == GUI_DIM_FITCONTENT) {
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
    if (text_element->height_dim == GUI_DIM_FITCONTENT) {
        set_draw_height(text_element, subdata->font_size * (subdata->num_lines * (BF_CHAR_HEIGHT + 1) - 1));
    }
}

GUI_Dimension get_container_sig_dim(GUI_Element *container) {
    return choriz(container) ? container->width_dim : container->height_dim;
}

GUI_Dimension get_container_non_sig_dim(GUI_Element *container) {
    return choriz(container) ? container->height_dim : container->width_dim;
}

void set_container_sig_dim_draw(GUI_Element *container, Uint16 value) {
    choriz(container) ? set_draw_width(container, value) : set_draw_height(container, value);
}

void set_container_non_sig_dim_draw(GUI_Element *container, Uint16 value) {
    choriz(container) ? set_draw_height(container, value) : set_draw_width(container, value);
}

GUI_Dimension get_container_element_non_sig_dim(GUI_Element *container, GUI_Element *element) {
    return choriz(container) ? element->height_dim : element->width_dim;
}

void set_container_element_non_sig_dim(GUI_Element *container, GUI_Element *element, GUI_Dimension dim) {
    choriz(container) ? (element->height_dim = dim) : (element->width_dim = dim);
}

Uint16 get_container_element_non_sig_dim_total_value(GUI_Element *container, GUI_Element *element) {
    return choriz(container) ? get_total_height(element) : get_total_width(element);
}

void set_container_element_non_sig_dim_total_value(GUI_Element *container, GUI_Element *element, Uint16 value) {
    choriz(container) ? set_total_height(element, value) : set_total_width(element, value);
}

Uint16 get_container_elements_sig_dim_value(GUI_Element *container) {
    return choriz(container) ? get_container_elements_width(container) : get_container_elements_height(container);
}

void expand_container_sig_dim_elements(GUI_Element *container) {
    GUI_EContainer *subdata = container->subdata;

    // Sum the significant dimension of elements that aren't set to expand
    // Also, get all expanding elements
    GUI_Element *expand_elements[subdata->num_expand_sig_dim_elements];
    int num_expand_elements_found = 0;
    Uint16 non_expand_extent = 0;
    for (GUI_EContainerNode *node = subdata->head; node; node = node->next) {
        if (is_expand_container_sig_dim(container, node->element)) {
            expand_elements[num_expand_elements_found++] = node->element;
        } else {
            non_expand_extent += subdata->is_horizontal ? node->element->width : node->element->height;
        }

        non_expand_extent += node->margin_back + node->margin_front + node->element->border_width * 2;
    }

    // Get the extent that each expanding element needs to fill
    Uint16 expand_extent = 
        ((choriz(container) ? get_draw_width(container) : get_draw_height(container)) - non_expand_extent)
        / subdata->num_expand_sig_dim_elements;
    
    // Set the proper significant dimension extent for each expanding element
    for (int i = 0; i < subdata->num_expand_sig_dim_elements; i++) {
        if (subdata->is_horizontal) {
            expand_elements[i]->width = expand_extent;
        } else {
            expand_elements[i]->height = expand_extent;
        }
        GUI_UpdateElementStatic(expand_elements[i]);
    }
}

void expand_container_non_sig_dim_elements(GUI_Element *container) {
    // For each element expanding on the non-significant dimension, set its dimension to be that of the container
    Uint16 container_draw = (choriz(container) ? get_draw_height(container) : get_draw_width(container));
    for (GUI_EContainerNode *node = cdata(container)->head; node; node = node->next) {
        if (get_container_element_non_sig_dim(container, node->element) == GUI_DIM_EXPAND) {
            choriz(container) ? set_total_height(node->element, container_draw) : set_total_width(node->element, container_draw);
            GUI_UpdateElementStatic(node->element);
        }
    }
}

void fit_container_sig_dim(GUI_Element *container) {
    set_container_sig_dim_draw(container, get_container_elements_sig_dim_value(container));
}

void fit_container_non_sig_dim(GUI_Element *container) {
    GUI_EContainer *subdata = container->subdata;

    if (subdata->num_elements == 0) {
        set_container_sig_dim_draw(container, 0);
    } else {
        // Get all elements with non-significant dimension set to expand to fit
        GUI_Element *expanding[subdata->num_expand_non_sig_dim_elements];
        int num_expanding = 0;

        // Get the max non-significant dimension value of all the elements
        Uint16 max_non_sig_value = 0;
        for (GUI_EContainerNode *node = subdata->head; node; node = node->next) {
            bool is_non_sig_expanding = (get_container_element_non_sig_dim(container, node->element) == GUI_DIM_EXPAND);
            if (is_non_sig_expanding) {
                // If this element expands in the non-significant direction, save it in an array
                expanding[num_expanding++] = node->element;

                // Get its height as its content height
                set_container_element_non_sig_dim(container, node->element, GUI_DIM_FITCONTENT);
                GUI_UpdateElementStatic(node->element);
            }

            Uint16 height = get_container_element_non_sig_dim_total_value(container, node->element);
            if (height > max_non_sig_value) {
                max_non_sig_value = height;
            }

            // Reset expanding element
            if (is_non_sig_expanding) {
                set_container_element_non_sig_dim(container, node->element, GUI_DIM_EXPAND);
            }
        }

        set_container_non_sig_dim_draw(container, max_non_sig_value);

        // Set all the non-sig-expanding elements' non-significant dimension value to the max
        if (num_expanding != 0) {
            for (int i = 0; i < num_expanding; i++) {
                set_container_element_non_sig_dim_total_value(container, expanding[i], max_non_sig_value);
            }
        }
    }
}

void update_container_data(GUI_Element *container) {
    // Handle updating along significant dimension
    if (get_container_sig_dim(container) == GUI_DIM_FITCONTENT) {
        fit_container_sig_dim(container);
    } else if (cdata(container)->num_expand_sig_dim_elements != 0) {
        expand_container_sig_dim_elements(container);
    }

    // Handle fit content along non-significant dimension
    if (get_container_non_sig_dim(container) == GUI_DIM_FITCONTENT) {
        fit_container_non_sig_dim(container);
    } else if (cdata(container)->num_expand_non_sig_dim_elements != 0) {
        expand_container_non_sig_dim_elements(container);
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