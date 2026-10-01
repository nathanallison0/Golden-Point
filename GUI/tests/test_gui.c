#define WINDOW_WIDTH (800 / 2)
#define WINDOW_HEIGHT (450 / 2)
#include "../include/gui.h"
#include <stdio.h>

GUI_Element *text_element;
GUI_EText *subdata;

void set_padding(Uint16 padding) {
    text_element->padding_top = padding;
    text_element->padding_right = padding;
    text_element->padding_bottom = padding;
    text_element->padding_left = padding;
}

void render(void) {
    clear_renderer();
    GUI_RenderElement(text_element, 0, 0);
    present_window();
}

bool element_frozen = false;
const bool *state;
bool prev_state[SDL_SCANCODE_COUNT];
#define key(k) SDL_SCANCODE_ ## k
#define pressed(k) state[key(k)]
#define just_pressed(k) state[key(k)] && !prev_state[key(k)]
int main() {
    state = SDL_GetKeyboardState(NULL);

    #define BORDER_WIDTH 4
    text_element = GUI_CreateElementText(
        &(GUI_Element) {
            .width = WINDOW_WIDTH - (BORDER_WIDTH * 2),
            .height = WINDOW_HEIGHT - (BORDER_WIDTH * 2),
            .fill_color = A_WHITE,
            .border_width = BORDER_WIDTH,
            .border_color = A_RED,
            .center_y = GUI_CENTERY_TOP,
            .center_x = GUI_CENTERX_LEFT,
            .padding_top = 0,
            .padding_right = 0,
            .padding_bottom = 0,
            .padding_left = 0
        },
        &(GUI_EText) {
            .font_color = A_BLACK,
            .font_size = 2,
            .text = "I'm finally starting to do java in my CS course. Finally. I fucking hate python"
        }
    );
    subdata = text_element->subdata;

    if (!initialize_window(SDL_INIT_VIDEO, "Test GUI", WINDOW_WIDTH, WINDOW_HEIGHT)) {
        fprintf(stderr, "Could not initalize sdl\n");
        GUI_DestroyElement(text_element);
    }

    render();

    SDL_Event event;
    bool waiting = true;
    while (waiting) {
        SDL_Delay(10);

        bool changed = false;
        while (waiting && SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_EVENT_QUIT:
                    waiting = false;
                    break;
                case SDL_EVENT_MOUSE_BUTTON_DOWN:
                    if (event.button.button == SDL_BUTTON_LEFT) {
                        element_frozen = !element_frozen;
                    }
                    if (element_frozen) {
                        break;
                    }
                case SDL_EVENT_MOUSE_MOTION:
                    if (!element_frozen) {
                        text_element->width = event.motion.x - text_element->border_width * 2;
                        text_element->height = event.motion.y - text_element->border_width * 2;
                        changed = true;
                    }
                    break;
            }
        }

        if (pressed(ESCAPE)) {
            waiting = false;
        }

        if (just_pressed(EQUALS)) {
            subdata->font_size++;
            changed = true;
        }

        if (just_pressed(MINUS)) {
            subdata->font_size--;
            changed = true;
        }
        
        #define pad_incr 10
        if (just_pressed(LEFTBRACKET)) {
            set_padding(text_element->padding_top - pad_incr);
            changed = true;
        }

        if (just_pressed(RIGHTBRACKET)) {
            set_padding(text_element->padding_top + pad_incr);
            changed = true;
        }

        #define border_incr 2
        if (just_pressed(DOWN)) {
            text_element->border_width -= border_incr;
            changed = true;
        }

        if (just_pressed(UP)) {
            text_element->border_width += border_incr;
            changed = true;
        }

        if (just_pressed(1)) {
            text_element->center_y = GUI_CENTERY_TOP;
            changed = true;
        }

        if (just_pressed(2)) {
            text_element->center_y = GUI_CENTERY_MIDDLE;
            changed = true;
        }

        if (just_pressed(3)) {
            text_element->center_y = GUI_CENTERY_BOTTOM;
            changed = true;
        }

        if (just_pressed(F1)) {
            text_element->center_x = GUI_CENTERX_LEFT;
            changed = true;
        }

        if (just_pressed(F2)) {
            text_element->center_x = GUI_CENTERX_MIDDLE;
            changed = true;
        }

        if (just_pressed(F3)) {
            text_element->center_x = GUI_CENTERX_RIGHT;
            changed = true;
        }

        memcpy(prev_state, state, sizeof(prev_state));

        if (changed) {
            render();
        }
    }

    destroy_window();
    GUI_DestroyElement(text_element);
}