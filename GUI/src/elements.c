#include "../include/elements.h"
#include <stdlib.h>
#include <stdio.h>

#include "../../../SDL/start/include/SDL3Start.h"

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

    memcpy(text_element, text_fields, sizeof(*text_element));
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

void (*destroy_data_functions[])(GUI_Element *) = {
    NULL, // notype
    NULL, // text
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