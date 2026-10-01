#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 450
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

const bool *state;
bool prev_state[SDL_SCANCODE_COUNT];
#define key(k) SDL_SCANCODE_ ## k
#define just_pressed(k) state[key(k)] && !prev_state[key(k)]
int main() {
    state = SDL_GetKeyboardState(NULL);

    text_element = GUI_CreateElementText(
        &(GUI_Element) {
            .width = 400,
            .height = 400,
            .fill_color = A_WHITE,
            .border_width = 4,
            .border_color = A_RED,
            .center_y = GUI_CENTERY_TOP,
            .center_x = GUI_CENTERX_LEFT,
            .padding_top = 20,
            .padding_right = 20,
            .padding_bottom = 20,
            .padding_left = 20
        },
        &(GUI_EText) {
            .font_color = A_BLACK,
            .font_size = 6,
            .text = "This is a header.\n\nHere's some crap about that header."
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
                case SDL_EVENT_MOUSE_MOTION:
                    text_element->width = event.motion.x - text_element->border_width * 2;
                    changed = true;
                    break;
            }
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

        memcpy(prev_state, state, sizeof(prev_state));

        if (changed) {
            render();
        }
    }

    destroy_window();
    GUI_DestroyElement(text_element);
}