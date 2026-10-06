#define WINDOW_WIDTH (800)
#define WINDOW_HEIGHT (450)
#include "../include/gui.h"
#include <stdio.h>

GUI_Element *root;

void render(void) {
    clear_renderer();
    GUI_RenderElement(root, 0, 0);
    present_window();
}

#define pad(x) 0

const bool *state;
bool prev_state[SDL_SCANCODE_COUNT];
#define key(k) SDL_SCANCODE_ ## k
#define pressed(k) state[key(k)]
#define just_pressed(k) state[key(k)] && !prev_state[key(k)]
int main() {
    state = SDL_GetKeyboardState(NULL);

    #define text_centerx GUI_CENTERX_LEFT
    #define text_centery GUI_CENTERY_TOP
    #define text_pad pad(10)

    GUI_Element text_style = {
        //.width = 350,
        .width_auto = true,
        //.height = 75,
        .height_auto = true,
        .fill_color = A_WHITE,
        .border_width = 2,
        .border_color = A_RED,
        .center_x = text_centerx,
        .center_y = text_centery,
        .padding_top = text_pad,
        .padding_right = text_pad,
        .padding_bottom = text_pad,
        .padding_left = text_pad
    };

    GUI_Element *text_element1 = GUI_CreateElementText(
        &text_style,
        &(GUI_EText) {
            .font_color = A_BLACK,
            .font_size = 3,
            .text = "Wow this is\nsome text"
        }
    );

    GUI_Element *text_element2 = GUI_CreateElementText(
        &text_style,
        &(GUI_EText) {
            .font_color = A_BLACK,
            .font_size = 3,
            .text = "Wow second text"
        }
    );

    GUI_Element *text_element3 = GUI_CreateElementText(
        &text_style,
        &(GUI_EText) {
            .font_color = A_BLACK,
            .font_size = 3,
            .text = "This text is doubly contained"
        }
    );

    GUI_Element *inner_container = GUI_CreateElementContainer(
        &(GUI_Element) {
            .width = 750,
            .height = 150,
            .fill_color = A_BLACK,
            .border_width = 3,
            .border_color = A_GREEN,
            .center_x = GUI_CENTERX_LEFT,
            .center_y = GUI_CENTERY_TOP,
            .padding_top = 0,
            .padding_right = 0,
            .padding_bottom = 0,
            .padding_left = 0
        },
        true
    );

    GUI_ContainerAddElement(inner_container, text_element3, 0, 0);

    #define BORDER 4
    #define container_pad 0//pad(20)
    root = GUI_CreateElementContainer(
        &(GUI_Element) {
            .width = WINDOW_WIDTH - (BORDER * 2),
            .height = WINDOW_HEIGHT - (BORDER * 2),
            .fill_color = A_WHITE,
            .border_width = BORDER,
            .border_color = A_BLUE,
            .center_x = GUI_CENTERX_LEFT,
            .center_y = GUI_CENTERY_TOP,
            .padding_top = container_pad,
            .padding_right = container_pad,
            .padding_bottom = container_pad,
            .padding_left = container_pad,
        },
        false
    );

    #define container_margin pad(5)
    GUI_ContainerAddElement(root, inner_container, container_margin, container_margin);
    GUI_ContainerAddElement(root, text_element2, container_margin, container_margin);
    GUI_ContainerAddElement(root, text_element1, container_margin, container_margin);
    

    if (!initialize_window(SDL_INIT_VIDEO, "Test GUI", WINDOW_WIDTH, WINDOW_HEIGHT)) {
        fprintf(stderr, "Could not initalize sdl\n");
        GUI_DestroyElement(root);
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
            }
        }

        if (pressed(ESCAPE)) {
            waiting = false;
        }

        memcpy(prev_state, state, sizeof(prev_state));

        if (changed) {
            render();
        }
    }

    destroy_window();
    GUI_DestroyElement(root);
}