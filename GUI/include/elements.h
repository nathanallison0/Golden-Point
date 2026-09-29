#ifndef _GPOINT_GUI_ELEMENTS_H_
#define _GPOINT_GUI_ELEMENTS_H_

#include "../../../SDL/start/include/SDL3Start.h"

enum {
    GUI_CENTER_TOP,
    GUI_CENTER_TOP_RIGHT,
    GUI_CENTER_RIGHT,
    GUI_CENTER_BOTTOM_RIGHT,
    GUI_CENTER_BOTTOM,
    GUI_CENTER_BOTTOM_LEFT,
    GUI_CENTER_LEFT,
    GUI_CENTER_TOP_LEFT,
    GUI_CENTER_MIDDLE
};
typedef Uint8 GUI_CenterContent;

enum {
    GUI_ETYPE_NONE,
    GUI_ETYPE_TEXT,
    GUI_ETYPE_CONTAINER
};
typedef Uint8 GUI_EType;

typedef struct GUI_Element {
    Uint16 width;
    Uint16 height;
    rgba fill_color;

    Uint8 border_width;
    rgba border_color;

    Uint16 padding_top;
    Uint16 padding_right;
    Uint16 padding_bottom;
    Uint16 padding_left;

    GUI_CenterContent center_content;

    GUI_EType type;
    void *subdata;
} GUI_Element;

typedef struct GUI_EText {
    char *text;
    Uint8 font_size;
    rgba font_color;
} GUI_EText;

/**
 * Creates a text element.
 * 
 * \param base_fields Element parent fields
 * \param text_fields Text element specific fields
 * \return A new text element
 */
GUI_Element *GUI_CreateElementText(GUI_Element *base_fields, GUI_EText *text_fields);

/**
 * Removes an element from memory.
 * 
 * \param element The element to destroy
 */
void GUI_DestroyElement(GUI_Element *element);

#endif