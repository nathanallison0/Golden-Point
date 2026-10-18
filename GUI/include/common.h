#ifndef _GPOINT_GUI_COMMON_H_
#define _GPOINT_GUI_COMMON_H_

#include "element.h"

#define subdata(element, type) ((type *) element->subdata)
#define tdata(text) subdata(text, GUI_EText)
#define cdata(container) subdata(container, GUI_EContainer)
#define choriz(container) (cdata(container)->is_horizontal)

Uint16 get_draw_width(GUI_Element *element);
Uint16 get_draw_height(GUI_Element *element);

void set_draw_width(GUI_Element *element, Uint16 draw_width);
void set_draw_height(GUI_Element *element, Uint16 draw_height);

Uint16 get_total_width(GUI_Element *element);
Uint16 get_total_height(GUI_Element *element);

void set_total_width(GUI_Element *element, Uint16 total_width);
void set_total_height(GUI_Element *element, Uint16 total_height);

Uint16 get_container_elements_width(GUI_Element *container);
Uint16 get_container_elements_height(GUI_Element *container);

bool is_expand_container_sig_dim(GUI_Element *container, GUI_Element *element);
bool is_expand_container_non_sig_dim(GUI_Element *container, GUI_Element *element);

#endif