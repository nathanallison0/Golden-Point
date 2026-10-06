#ifndef _GPOINT_GUI_COMMON_H_
#define _GPOINT_GUI_COMMON_H_

#include "elements.h"

Uint16 get_draw_width(GUI_Element *element);
Uint16 get_draw_height(GUI_Element *element);

void set_draw_width(GUI_Element *element, Uint16 draw_width);
void set_draw_height(GUI_Element *element, Uint16 draw_height);

#endif