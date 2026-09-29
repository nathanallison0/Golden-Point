#include "../include/elements.h"
#include <stdlib.h>
#include <stdio.h>

#include "../../../SDL/start/include/SDL3Start.h"

GUI_Element *GUI_CreateElement(GUI_Element *fields) {
    GUI_Element *element = malloc(sizeof(*element));
    if (!element) {
        return NULL;
    }

    // Allocate all but subdata 
    memcpy(element, fields, sizeof(*element));
    return element;
}

GUI_Element *GUI_CreateElementText(GUI_Element *base_fields, GUI_EText *text_fields) {
    GUI_EText *text_element = malloc(sizeof(*text_element));
    if (!text_element) {
        fprintf(stderr, "Could not allocate space for text fields of GUI text element\n");
        return NULL;
    }

    memcpy(text_element, text_fields, sizeof(*text_element));

    GUI_Element *base_element = GUI_CreateElement(base_fields);
    if (!base_element) {
        fprintf(stderr, "Could not allocate space for base fields of GUI text element\n");
        free(text_element);
        return NULL;
    }

    base_element->type = GUI_ETYPE_TEXT;
    base_element->subdata = text_element;
    return base_element;
}

void GUI_DestroyElement(GUI_Element *element) {
    if (element->subdata) {
        free(element->subdata);
    }
    free(element);
}