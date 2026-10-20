#define WINDOW_WIDTH (800)
#define WINDOW_HEIGHT (450)
#include "../include/gui.h"
#include "../include/common.h"
#include <stdio.h>

GUI_Element *root;

void render(void) {
    clear_renderer();
    GUI_RenderElement(root, 0, 0);
    present_window();
}

#define pad(x) x

const bool *state;
bool prev_state[SDL_SCANCODE_COUNT];
#define key(k) SDL_SCANCODE_ ## k
#define pressed(k) state[key(k)]
#define just_pressed(k) state[key(k)] && !prev_state[key(k)]
int main() {
    state = SDL_GetKeyboardState(NULL);

    #define text_pad pad(10)

    GUI_Element text_style = {
        //.width = 500,
        .width_dim = GUI_DIM_EXPAND,
        //.height = 75,
        .height_dim = GUI_DIM_EXPAND,
        .fill_color = A_WHITE,
        .border_width = 2,
        .border_color = A_RED,
        .center_x = GUI_CENTERX_MIDDLE,
        .center_y = GUI_CENTERY_MIDDLE,
        .padding_top = text_pad,
        .padding_right = text_pad,
        .padding_bottom = text_pad,
        .padding_left = text_pad
    };

    char sentence[] = "Lorem ipsum dolor sit amet consectetur adipiscing elit quisque faucibus ex sapien vitae pellentesque sem placerat in id cursus mi.";
    #define NUM_WORDS 20
    int words_shown = NUM_WORDS;
    GUI_Element *text_element1 = GUI_CreateElementText(
        &text_style,
        &(GUI_EText) {
            .font_color = A_BLACK,
            .font_size = 3,
            .text = sentence
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
            .text = "This text is doubly contained and expands"
        }
    );

    GUI_Element *text_element4 = GUI_CreateElementText(
        &(GUI_Element) {
            //.width = 300,
            .width_dim = GUI_DIM_EXPAND,
            //.height = 200,
            .height_dim = GUI_DIM_EXPAND,
            .fill_color = A_WHITE,
            .border_width = 2,
            .border_color = A_RED,
            .center_x = GUI_CENTERX_MIDDLE,
            .center_y = GUI_CENTERY_MIDDLE,
            .padding_top = text_pad,
            .padding_right = text_pad,
            .padding_bottom = text_pad,
            .padding_left = text_pad
        },
        &(GUI_EText) {
            .font_color = A_BLACK,
            .font_size = 3,
            .text = "The fourth\ntext is\nover here"
        }
    );

    GUI_Element *inner_container = GUI_CreateElementContainer(
        &(GUI_Element) {
            //.width = 750,
            .width_dim = GUI_DIM_EXPAND,
            //.height = 150,
            .height_dim = GUI_DIM_EXPAND,
            .fill_color = A_WHITE,
            .border_width = 0,
            .border_color = A_GREEN,
            .center_x = GUI_CENTERX_MIDDLE,
            .center_y = GUI_CENTERY_MIDDLE,
            .padding_top = 0,
            .padding_right = 0,
            .padding_bottom = 0,
            .padding_left = 0
        },
        true
    );

    #define container_margin pad(5)

    GUI_ContainerAddElement(inner_container, text_element4, container_margin, 0);
    GUI_ContainerAddElement(inner_container, text_element3, 0, container_margin);

    #define BORDER 4
    #define container_pad pad(20)
    #define HEIGHT (WINDOW_HEIGHT - (BORDER * 2))
    root = GUI_CreateElementContainer(
        &(GUI_Element) {
            .width = WINDOW_WIDTH - (BORDER * 2),
            .height = HEIGHT,
            .fill_color = A_WHITE,
            .border_width = BORDER,
            .border_color = A_BLUE,
            .center_x = GUI_CENTERX_MIDDLE,
            .center_y = GUI_CENTERY_MIDDLE,
            .padding_top = container_pad,
            .padding_right = container_pad,
            .padding_bottom = container_pad,
            .padding_left = container_pad,
        },
        false
    );

    GUI_ContainerAddElement(root, inner_container, container_margin, container_margin);
    GUI_ContainerAddElement(root, text_element2, container_margin, container_margin);
    GUI_ContainerAddElement(root, text_element1, container_margin, container_margin);

    if (!initialize_window(SDL_INIT_VIDEO, "Test GUI", WINDOW_WIDTH, WINDOW_HEIGHT)) {
        fprintf(stderr, "Could not initalize sdl\n");
        GUI_DestroyElement(root);
    }

    render();

    SDL_Event event;
    bool running = true;
    while (running) {
        SDL_Delay(10);

        bool changed = false;
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_EVENT_QUIT:
                    running = false;
                    break;
            }
        }

        if (pressed(ESCAPE)) {
            running = false;
        }

        if (just_pressed(EQUALS)) {
            if (words_shown < NUM_WORDS) {
                *strchr(sentence, '\0') = ' ';
                words_shown++;
                changed = true;
            }
        }

        if (just_pressed(MINUS)) {
            if (words_shown > 1) {
                *strrchr(sentence, ' ') = '\0';
                words_shown--;
                changed = true;
            }
        }

        memcpy(prev_state, state, sizeof(prev_state));

        if (changed) {
            GUI_UpdateElement(text_element1);
            render();
        }
    }

    destroy_window();
    GUI_DestroyElement(root);
}