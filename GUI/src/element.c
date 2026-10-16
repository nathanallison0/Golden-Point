#include "../include/element.h"
#include "../include/update.h"
#include "../include/common.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "../../../SDL/start/include/SDL3Start.h"

const char *type_names[GUI_NUM_ETYPES] = {
    "NONE",
    "TEXT",
    "CONTAINER"
};

const char *GUI_GetTypeName(GUI_EType type) {
    return type_names[type];
}

GUI_Element *GUI_CreateElement(GUI_Element *fields, void *subdata) {
    GUI_Element *element = malloc(sizeof(*element));
    if (!element) {
        return NULL;
    }

    memcpy(element, fields, sizeof(*element));
    element->subdata = subdata;

    // If a dimension is set to fit content, set the centering to be default
    // to save unnecessary calculations
    if (element->width_dim == GUI_DIM_FITCONTENT) {
        element->center_x = GUI_CENTERX_LEFT;
    }

    if (element->height_dim == GUI_DIM_FITCONTENT) {
        element->center_y = GUI_CENTERY_TOP;
    }

    return element;
}

GUI_Element *GUI_CreateElementText(GUI_Element *base_fields, GUI_EText *text_fields) {
    GUI_EText *text_element = malloc(sizeof(*text_fields));
    if (!text_element) {
        fprintf(stderr, "Could not allocate space for text fields of GUI text element\n");
        return NULL;
    }

    GUI_Element *base_element = GUI_CreateElement(base_fields, text_element);
    if (!base_element) {
        fprintf(stderr, "Could not allocate space for base fields of GUI text element\n");
        free(text_fields);
        return NULL;
    }

    base_element->type = GUI_ETYPE_TEXT;
    text_element->formatted_text = NULL;
    text_element->num_lines = 0;

    memcpy(text_element, text_fields, sizeof(*text_fields));

    GUI_UpdateElementStatic(base_element);

    return base_element;
}

GUI_Element *GUI_CreateElementContainer(GUI_Element *base_fields, bool is_horizontal) {
    GUI_EContainer *container_fields = malloc(sizeof(*container_fields));
    if (!container_fields) {
        fprintf(stderr, "Could not allocate space for container fields of GUI container element\n");
        return NULL;
    }

    GUI_Element *base_element = GUI_CreateElement(base_fields, container_fields);
    if (!base_element) {
        fprintf(stderr, "Could not allocate space for base fields of GUI container element\n");
        free(container_fields);
        return NULL;
    }

    base_element->type = GUI_ETYPE_CONTAINER;

    container_fields->head = NULL;
    container_fields->tail = NULL;
    container_fields->num_elements = 0;
    container_fields->num_expand_sig_dim_elements = 0;
    container_fields->is_horizontal = is_horizontal;

    return base_element;
}

void destroy_text_data(GUI_Element *text_fields) {
    GUI_EText *subdata = text_fields->subdata;
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