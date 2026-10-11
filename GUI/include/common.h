#ifndef _GPOINT_GUI_COMMON_H_
#define _GPOINT_GUI_COMMON_H_

#include "element.h"

Uint16 get_draw_width(GUI_Element *element);
Uint16 get_draw_height(GUI_Element *element);

void set_draw_width(GUI_Element *element, Uint16 draw_width);
void set_draw_height(GUI_Element *element, Uint16 draw_height);

Uint16 get_total_width(GUI_Element *element);
Uint16 get_total_height(GUI_Element *element);

Uint16 get_container_elements_width(GUI_Element *container);
Uint16 get_container_elements_height(GUI_Element *container);

#endif