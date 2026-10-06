#include "../include/elements.h"
#include "../include/common.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "../../../SDL/start/include/SDL3Start.h"

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

}

void (*update_functions[GUI_NUM_ETYPES])(GUI_Element *) = {
    NULL, // notype
    update_text_data,
    update_container_data
};

void GUI_UpdateElement(GUI_Element *element) {
    if (update_functions[element->type]) {
        update_functions[element->type](element);
    }
}

GUI_Element *GUI_CreateElement(GUI_Element *fields) {
    GUI_Element *element = malloc(sizeof(*element));
    if (!element) {
        return NULL;
    }

    memcpy(element, fields, sizeof(*element));
    return element;
}

GUI_Element *GUI_CreateElementText(GUI_Element *base_fields, GUI_EText *text_fields) {
    GUI_EText *text_element = malloc(sizeof(*text_element));
    if (!text_element) {
        fprintf(stderr, "Could not allocate space for text fields of GUI text element\n");
        return NULL;
    }

    GUI_Element *base_element = GUI_CreateElement(base_fields);
    if (!base_element) {
        fprintf(stderr, "Could not allocate space for base fields of GUI text element\n");
        free(text_element);
        return NULL;
    }

    base_element->type = GUI_ETYPE_TEXT;
    base_element->subdata = text_element;
    text_element->formatted_text = NULL;
    text_element->num_lines = 0;

    memcpy(text_element, text_fields, sizeof(*text_element));

    update_text_data(base_element);

    return base_element;
}

GUI_Element *GUI_CreateElementContainer(GUI_Element *base_fields, bool is_horizontal) {
    GUI_EContainer *container_element = malloc(sizeof(*container_element));
    if (!container_element) {
        fprintf(stderr, "Could not allocate space for container fields of GUI container element\n");
        return NULL;
    }

    GUI_Element *base_element = GUI_CreateElement(base_fields);
    if (!base_element) {
        fprintf(stderr, "Could not allocate space for base fields of GUI container element\n");
        free(container_element);
        return NULL;
    }

    base_element->type = GUI_ETYPE_CONTAINER;
    base_element->subdata = container_element;

    container_element->head = NULL;
    container_element->tail = NULL;
    container_element->num_elements = 0;
    container_element->is_horizontal = is_horizontal;

    return base_element;
}

GUI_EContainerNode *create_container_node(GUI_Element *element, Uint16 margin_back, Uint16 margin_front) {
    GUI_EContainerNode *node = malloc(sizeof(*node));
    if (!node) {
        fprintf(stderr, "Could not allocate space for GUI container element node\n");
        return NULL;
    }

    node->element = element;
    node->margin_back = margin_back;
    node->margin_front = margin_front;

    return node;
}

void GUI_ContainerAddElement(GUI_Element *container, GUI_Element *element, Uint16 margin_back, Uint16 margin_front) {
    GUI_EContainerNode *node = create_container_node(element, margin_back, margin_front);
    if (!node) {
        return;
    }

    GUI_EContainer *subdata = container->subdata;
    if (subdata->head) {
        node->next = subdata->head;
        subdata->head->prev = node;
    } else {
        node->next = NULL;
        subdata->tail = node;
    }
    subdata->head = node;
    node->prev = NULL;

    subdata->num_elements++;
}

void destroy_text_data(GUI_Element *text_element) {
    GUI_EText *subdata = text_element->subdata;
    if (subdata->formatted_text) {
        alstring_destroy(subdata->formatted_text);
    }
}

void destroy_container_data(GUI_Element *container) {
    GUI_EContainer *subdata = container->subdata;
    GUI_EContainerNode *node = subdata->head;
    GUI_EContainerNode *next;
    while (node) {
        next = node->next;

        GUI_DestroyElement(node->element);
        free(node);
        node = next;
    }
}

void (*destroy_data_functions[GUI_NUM_ETYPES])(GUI_Element *) = {
    NULL, // notype
    destroy_text_data, // text
    destroy_container_data
};

void GUI_DestroyElement(GUI_Element *element) {
    // If element has a destroyer, call
    if (destroy_data_functions[element->type]) {
        destroy_data_functions[element->type](element);
    }

    if (element->subdata) {
        free(element->subdata);
    }
    free(element);
}