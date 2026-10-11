#ifndef _GPOINT_GUI_UPDATE_H_
#define _GPOINT_GUI_UPDATE_H_

#include "element.h"

/**
 * Updates an element to match changes to its content without
 * updating its container.
 * 
 * \param element The element to update
 */
void GUI_UpdateElementStatic(GUI_Element *element);

/**
 * Updates an element to match changes to its content.
 * 
 * \param element The element to update
 */
void GUI_UpdateElement(GUI_Element *element);

#endif