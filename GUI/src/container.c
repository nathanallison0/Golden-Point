#include "../include/container.h"
#include "../include/update.h"
#include "../include/common.h"
#include "stdlib.h"

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
    if (is_expand_container_sig_dim(container, element)) {
        subdata->num_expand_sig_dim_elements++;
    }
    if (is_expand_container_non_sig_dim(container, element)) {
        subdata->num_expand_non_sig_dim_elements++;
    }

    GUI_UpdateElement(container);
}