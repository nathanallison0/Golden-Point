#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 450
#include "../include/gui.h"
#include <stdio.h>

int main() {
    GUI_Element *text_element = GUI_CreateElementText(
        &(GUI_Element) {
            .border_color = A_RED,
            .border_width = 4,
            .center_content = GUI_CENTER_TOP_LEFT,
            .fill_color = A_WHITE,
            .height = 400,
            .padding_bottom = 20,
            .padding_left = 0,
            .padding_right = 20,
            .padding_top = 0,
            .width = 400
        },
        &(GUI_EText) {
            .font_color = A_BLACK,
            .font_size = 8,
            .text = "Hello World!"
        }
    );

    if (!initialize_window(SDL_INIT_VIDEO, "Test GUI", WINDOW_WIDTH, WINDOW_HEIGHT)) {
        fprintf(stderr, "Could not initalize sdl\n");
        GUI_DestroyElement(text_element);
    }

    clear_renderer();
    GUI_RenderElement(text_element, 0, 0);
    present_window();

    SDL_Event event;
    bool waiting = true;
    while (waiting) {
        while (waiting && SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_EVENT_QUIT:
                    waiting = false;
            }
        }

        SDL_Delay(10);
    }

    destroy_window();
    GUI_DestroyElement(text_element);
}