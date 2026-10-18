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
    GUI_DIM_STATIC,
    GUI_DIM_FITCONTENT,
    GUI_DIM_EXPAND
};
typedef Uint8 GUI_Dimension;

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
    GUI_Dimension width_dim;
    GUI_Dimension height_dim;
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
    Uint16 num_expand_sig_dim_elements;
    Uint16 num_expand_non_sig_dim_elements;
    bool is_horizontal;
} GUI_EContainer;

/**
 * Returns the name of an element type as a string.
 * 
 * \param type The element type
 * \return The name of the element type as a string
 */
const char *GUI_GetTypeName(GUI_EType type);

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
 * Removes an element from memory.
 * 
 * \param element The element to destroy
 */
void GUI_DestroyElement(GUI_Element *element);

#endif