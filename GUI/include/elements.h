#ifndef _GPOINT_GUI_ELEMENTS_H_
#define _GPOINT_GUI_ELEMENTS_H_

#include "../../../SDL/start/include/SDL3Start.h"
#include "../../BasicFont/include/BasicFont.h"

enum {
    GUI_CENTERX_LEFT,
    GUI_CENTERX_MIDDLE,
    GUI_CENTERX_RIGHT
};
typedef Uint8 GUI_CenterContentX;

enum {
    GUI_CENTERY_TOP,
    GUI_CENTERY_MIDDLE,
    GUI_CENTERY_BOTTOM
};
typedef Uint8 GUI_CenterContentY;

enum {
    GUI_ETYPE_NONE,
    GUI_ETYPE_TEXT,
    GUI_ETYPE_CONTAINER,
    GUI_NUM_ETYPES
};
typedef Uint8 GUI_EType;

typedef struct GUI_Element {
    Uint16 width;
    Uint16 height;
    bool width_auto;
    bool height_auto;
    rgba fill_color;

    Uint8 border_width;
    rgba border_color;

    Uint16 padding_top;
    Uint16 padding_right;
    Uint16 padding_bottom;
    Uint16 padding_left;

    GUI_CenterContentX center_x;
    GUI_CenterContentY center_y;

    GUI_EType type;
    void *subdata;
} GUI_Element;

typedef struct GUI_EText {
    char *text;
    Uint8 font_size;
    rgba font_color;
    alstring *formatted_text;
    int num_lines;
} GUI_EText;

typedef struct GUI_EContainerNode {
    GUI_Element *element;
    struct GUI_EContainerNode *prev;
    struct GUI_EContainerNode *next;

    Uint16 margin_back;
    Uint16 margin_front;
} GUI_EContainerNode;

typedef struct GUI_EContainer {
    GUI_EContainerNode *head;
    GUI_EContainerNode *tail;
    Uint16 num_elements;
    bool is_horizontal;
} GUI_EContainer;

/**
 * Creates a text element.
 * 
 * \param base_fields Element parent fields
 * \param text_fields Text element specific fields
 * \return A new text element
 */
GUI_Element *GUI_CreateElementText(GUI_Element *base_fields, GUI_EText *text_fields);

/**
 * Creates a container element.
 * 
 * \param base_fields Element parent fields
 * \param is_horizontal If `true`, the container's elements will be placed horizontally, otherwise vertical
 * \return A new container element
 */
GUI_Element *GUI_CreateElementContainer(GUI_Element *base_fields, bool is_horizontal);

/**
 * Adds an element to a container at the first position.
 * 
 * \param container The container to add to
 * \param element The element to add
 * \param margin_back The back margin of the element
 * \param margin_front The front margin of the element
 */
void GUI_ContainerAddElement(GUI_Element *container, GUI_Element *element, Uint16 margin_back, Uint16 margin_front);

/**
 * Updates an element to match changes to its content.
 * 
 * \param element The element to update
 */
void GUI_UpdateElement(GUI_Element *element);

/**
 * Removes an element from memory.
 * 
 * \param element The element to destroy
 */
void GUI_DestroyElement(GUI_Element *element);

#endif