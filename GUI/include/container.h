#ifndef _GPOINT_GUI_CONTAINTER_H_
#define _GPOINT_GUI_CONTAINTER_H_

#include "element.h"

/**
 * Adds an element to a container at the first position.
 * 
 * \param container The container to add to
 * \param element The element to add
 * \param margin_back The back margin of the element
 * \param margin_front The front margin of the element
 */
void GUI_ContainerAddElement(GUI_Element *container, GUI_Element *element, Uint16 margin_back, Uint16 margin_front);

#endif