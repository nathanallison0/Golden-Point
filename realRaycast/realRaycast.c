#include <math.h>
#include <string.h>
#include <dirent.h>

// Web integration
#if __EMSCRIPTEN__
#include <emscripten.h>
#else
#include <SDL3_mixer/SDL_mixer.h>
#endif

#define FALSE 0
#define TRUE 1

#if __EMSCRIPTEN__
#define WINDOW_SCALE 30
#else
#define WINDOW_SCALE 80
#endif

#define WINDOW_WIDTH (16 * WINDOW_SCALE)
#define WINDOW_HEIGHT (9 * WINDOW_SCALE)

#define FPS 60
#define FRAME_TARGET_TIME (1000 / FPS)

#define SCHOOL_APPROPRIATE TRUE

#define GRID_SPACING 64
#define WORLDSPACE ((float) GRID_SPACING / WINDOW_HEIGHT)

#define assert(x) if (!(x)) { printf("assertion error on line %d - " #x ": '%s'\n", __LINE__, SDL_GetError()); exit(1); }

// Custom SDL graphics functions
// draw_rect(), etc.
#include "../../SDL/SDL3Start.h"

#define APPLY_BLUR FALSE
Uint8 aa_level = 1;

#define free(x) SDL_free(x)
#define malloc(x) SDL_malloc(x)
#define realloc(x, y) SDL_realloc(x, y)

// Font
#include "../BasicFont/BasicFont.h"

// Linked list generation
#include "../linkedList.h"

// Textures
#include "textures.h"

#include "graphics.h"
#if SCHOOL_APPROPRIATE
#define sprite_shot sprite_laserShot
#define image_weapon2ForGame image_laserForGame
#endif

#define WALL_TEXTURES TRUE
#define FLOOR_TEXTURES TRUE
#define SPRITES TRUE
#define SLOW_LIGHTING FALSE

#define fix_angle_360(angle) { if (angle < 0) angle += PI * 2; else if (angle >= PI * 2) angle -= PI * 2; }
#define fix_angle_180(angle) { if (angle < -PI) angle += PI * 2; else if (angle >= PI) angle -= PI * 2; }

#define range(left, sym1, var, sym2, right) (left sym1 var && var sym2 right)

// View modes
enum {
    VIEW_GRID,
    VIEW_FPS,
    VIEW_TERMINAL
};
typedef Uint8 view_mode;

view_mode view;
view_mode prev_view;
char fps_free_mouse = FALSE;

void focus_mouse(void) {
    SDL_SetWindowRelativeMouseMode(window, true);
}

void unfocus_mouse(void) {
    SDL_SetWindowRelativeMouseMode(window, false);
}

bool mouse_focused(void) {
    return SDL_GetWindowRelativeMouseMode(window);
}

void enter_fps_view(void) {
    view = VIEW_FPS;
    focus_mouse();
    fps_free_mouse = FALSE;
}

void enter_grid_view(void) {
    view = VIEW_GRID;
    unfocus_mouse();
}

void toggle_terminal_view(void) {
    if (view == VIEW_TERMINAL) {
        if (prev_view == VIEW_FPS) {
            enter_fps_view();
        } else {
            enter_grid_view();
        }
    } else {
        prev_view = view;
        view = VIEW_TERMINAL;
        unfocus_mouse();
    }
}

typedef struct {
    float x;
    float y;
} xy;

#include "map.h"

#if __EMSCRIPTEN__
Uint8 show_fps = FALSE;
#else
Uint8 show_fps = TRUE;
#endif

Uint32 frames = 0;
Uint8 cap_fps = TRUE;
float delta_time;
// Grid visual
rgb grid_bg = {255, 0, 255};
rgb grid_fill_nonsolid = {235, 235, 235};
rgb grid_fill_solid = {100, 110, 100};
int grid_line_width = 1;
rgb grid_line_fill = C_BLACK;
#define GRID_DOOR_THICKNESS 4
Uint8 show_grid_lines = FALSE;
// Grid Visual Camera
float grid_cam_x;
float grid_cam_y;
float grid_cam_zoom;
float grid_cam_zoom_incr = 0.025f;
float grid_cam_zoom_min = 0.1f;
float grid_cam_zoom_max = 1.25f;
float grid_cam_center_x;
float grid_cam_center_y;
Uint8 show_mouse_coords = FALSE;
float grid_mouse_x, grid_mouse_y;
Uint8 grid_casting = FALSE;
Uint8 debug_prr = FALSE;

// Player
float player_x;
float player_y;
float player_z;
float player_z_base;
#define PLAYER_Z_BOB_DISP -2.5f
#define PLAYER_Z_BOB_SPEED_FACTOR 2
int player_radius = 10;
float player_x_velocity;
float player_y_velocity;
int player_max_velocity = 200;
float player_angle;
float fov = PI / 3;
int player_movement_accel = 800;
int player_movement_decel = 800;
float player_rotation_speed = 4 * (PI / 9);
#if __EMSCRIPTEN__
float player_sensitivity = 0.15f;
#else
float player_sensitivity = 0.05f;
#endif

// Player interactions
#define KEY_OPEN_DOOR SDL_SCANCODE_E
#define OPEN_DOOR_DIST (GRID_SPACING * 2.5f)
#define DOOR_SPEED 100

// Player grid visual
int grid_player_pointer_dist = 45;
rgb grid_player_fill = {255, 50, 50};
Uint8 grid_follow_player = TRUE;
Uint8 show_player_vision = FALSE;
Uint8 show_player_trail = FALSE;

// Grid mobjs
int grid_mobj_radius = 7;

// First person rendering
float fp_scale = 1 / 0.009417f;
#define FP_RENDER_DISTANCE 2000
#define FP_BRIGHTNESS 0.9f

float fp_brightness_appl;
Uint8 fp_show_walls = TRUE;
int pixel_fov_circumference;
float radians_per_pixel;
Uint8 fp_show_weapon = TRUE;

float weapon_cycle_progress = 0;
#if __EMSCRIPTEN__
#define WEAPON_GRAPHIC_CYCLE_DISP_FACTOR 2
#else
#define WEAPON_GRAPHIC_CYCLE_DISP_FACTOR 5
#endif

#define WEAPON_GRAPHIC_CYCLE_LENGTH 1.5f
#define WEAPON_GRAPHIC_CYCLE_X_DISP (WEAPON_GRAPHIC_CYCLE_DISP_FACTOR * 15)
#define WEAPON_GRAPHIC_CYCLE_Y_DISP (WEAPON_GRAPHIC_CYCLE_DISP_FACTOR * 4)
#define WEAPON_GRAPHIC_CYCLE_SPEED_FACTOR 0.0025f

// User Input Variables
int shift = FALSE;
int left_mouse_down = FALSE;
float mouse_x;
float mouse_y;
float prev_mouse_x = -1;
float prev_mouse_y = -1;
char horizontal_input;
char vertical_input;
char rotation_input = 0;

rgb grid_mobj_color = C_BLUE;

__linked_list_all__(
    sprite_proj,
        float dist;
        float z;
        float angle;
        int sprite_num,
    (float dist, float z, float angle, int sprite_num),
        item->dist = dist;
        item->z = z;
        item->angle = angle;
        item->sprite_num = sprite_num
)

#include "./debugging.h"

// Grid Graphics
void g_draw_rect(float x, float y, float length, float width, Uint8 r, Uint8 g, Uint8 b) {
    draw_rect_f(
        (x - grid_cam_x) * grid_cam_zoom,
        (y - grid_cam_y) * grid_cam_zoom,
        length * grid_cam_zoom,
        width * grid_cam_zoom,
        r, g, b
    );
}

void g_draw_rect_rgb(float x, float y, float length, float width, rgb color) {
    g_draw_rect(x, y, length, width, color.r, color.g, color.b);
}

void g_draw_point(float x, float y, float radius, Uint8 r, Uint8 g, Uint8 b) {
    draw_point_f(
        (x - grid_cam_x) * grid_cam_zoom,
        (y - grid_cam_y) * grid_cam_zoom,
        radius,
        r, g, b
    );
}

void g_draw_point_rgb(float x, float y, float radius, rgb color) {
    g_draw_point(x, y, radius, color.r, color.g, color.b);
}

void g_draw_scale_point(float x, float y, float radius, Uint8 r, Uint8 g, Uint8 b) {
    g_draw_point(x, y, radius * grid_cam_zoom, r, g, b);
}

void g_draw_scale_point_rgb(float x, float y, float radius, rgb color) {
    g_draw_scale_point(x, y, radius, color.r, color.g, color.b);
}

// First person rendering
float point_dist(float x1, float y1, float x2, float y2) {
    return sqrtf(powf(x1 - x2, 2) + powf(y1 - y2, 2));
}

float fp_dist(float x, float y, float angle_to) {
    float offset = fmodf((player_angle - angle_to) + (PI * 2), PI * 2);
    if (offset > PI) {
        offset -= PI * 2;
    }
    offset = fabsf(offset);

    if (PI_4 < offset && offset < PI - PI_4) {
        offset -= PI_2;
    }

    return point_dist(x, y, player_x, player_y) * cosf(offset);
}

float angle_to_screen_x(float angle_to) {
    float relative_angle = angle_to - (player_angle - (fov / 2));
    fix_angle_360(relative_angle);
    return (WINDOW_WIDTH / fov) * relative_angle;
}

float player_height_y_offset(float height) {
    return -((height / 2) - ((player_z / GRID_SPACING) * height));
}

float project(float distance) {
    return WINDOW_HEIGHT / (distance / fp_scale);
}

#define get_horiz_texture(x, y) horiz_textures   [(int) (y) / GRID_SPACING][(int) (x) / GRID_SPACING]
#define get_vert_texture(x, y)  vertical_textures[(int) (y) / GRID_SPACING][(int) (x) / GRID_SPACING]

#define SHADE_DIST 1024
Uint8 shading_table[SHADE_DIST][256];
void precompute_shading_table(void) {
    for (int i = 0; i < SHADE_DIST; i++) {
        float dist = ((float) i / SHADE_DIST) * FP_RENDER_DISTANCE;
        for (int c = 0; c < 256; c++) {
            float shaded = (c - ((float) c / FP_RENDER_DISTANCE) * dist) * FP_BRIGHTNESS;
            if (shaded < 0) {
                shaded = 0;
            } else if (shaded > 255) {
                shaded = 255;
            }
            shading_table[i][c] = (Uint8) shaded;
        }
    }
}

Uint8 shade(Uint8 color, float distance) {
    int index = (distance / FP_RENDER_DISTANCE) * SHADE_DIST;
    return shading_table[index][color];
}

rgb shade_rgb(rgb color, float distance) {
    return (rgb) {
        shade(color.r, distance),
        shade(color.g, distance),
        shade(color.b, distance)
    };
}

rgba shade_rgba(rgba color, float distance) {
    return (rgba) {
        shade(color.r, distance),
        shade(color.g, distance),
        shade(color.b, distance),
        color.a
    };
}

float get_angle_to(float x1, float y1, float x2, float y2) {
    return atan2f(y2 - y1, x2 - x1);
}

float get_angle_from_player(float x, float y) {
    return get_angle_to(player_x, player_y, x, y);
}

float get_angle_to_player(float x, float y) {
    return get_angle_to(x, y, player_x, player_y);
}

#include "mobj.h"
#include "raycasts.h"
#include "sounds.h"
#include "shot.h"

void add_sprite_proj_a(float x, float y, float z, float angle_to, Uint16 sprite_num) {
    float distance = fp_dist(x, y, angle_to);
    if (distance > FP_RENDER_DISTANCE) {
        return;
    }

    // Store, sorted farthest to closest, for rendering
    sprite_proj *new_proj = sprite_proj_create(distance, z, angle_to, sprite_num);
    
    if (sprite_proj_head) {
        if (sprite_proj_head->dist < distance) {
            new_proj->next = sprite_proj_head;
            sprite_proj_head = new_proj;
        } else {
            // Find closest greater sprite
            sprite_proj *greater = sprite_proj_head;
            for (;greater->next && greater->next->dist > distance; greater = greater->next);

            // Insert this sprite after the lesser sprite
            sprite_proj *lesser = greater->next;
            greater->next = new_proj;
            new_proj->next = lesser;
        }
    } else {
        sprite_proj_head = new_proj;
    }
}

void add_sprite_proj(float x, float y, float z, Uint16 sprite_num) {
    add_sprite_proj_a(x, y, z, get_angle_from_player(x, y), sprite_num);
}

void add_sprite_proj_mobj(mobj *o) {
    if (sprites[o->sprite_index].is_rot) {
        float angle_to = get_angle_from_player(o->x, o->y);

        float relative_angle = o->angle - angle_to + PI - (ROT_SPRITE_INCR / 2);
        fix_angle_360(relative_angle);

        add_sprite_proj_a(o->x, o->y, o->z, angle_to, o->sprite_index + (NUM_ROT_SPRITE_FRAMES - (Uint8) (relative_angle / ROT_SPRITE_INCR) - 1));
    } else {
        add_sprite_proj(o->x, o->y, o->z, o->sprite_index);
    }
}

void calc_grid_cam_center(void) {
    grid_cam_center_x = grid_cam_x + ((WINDOW_WIDTH / 2) / grid_cam_zoom);
    grid_cam_center_y = grid_cam_y + ((WINDOW_HEIGHT / 2) / grid_cam_zoom);
}

// Resets
void reset_grid_cam(void) {
    grid_cam_x = 0;
    grid_cam_y = 0;
    grid_cam_zoom = 1;
}

void reset_player(void) {
    player_x = GRID_SPACING * 4;
    player_y = GRID_SPACING * 4;
    player_z_base = 40;
    player_x_velocity = 0;
    player_y_velocity = 0;
    player_angle = 0;
    weapon_cycle_progress = 0;
}

// Zoom grid
void zoom_grid_cam(float zoom) {
    grid_cam_zoom += zoom;
    if (grid_cam_zoom < grid_cam_zoom_min) {
        grid_cam_zoom = grid_cam_zoom_min;
    } else if (grid_cam_zoom > grid_cam_zoom_max) {
        grid_cam_zoom = grid_cam_zoom_max;
    }
}

void zoom_grid_cam_center(float zoom) {
    calc_grid_cam_center();
    float old_grid_cam_center_x = grid_cam_center_x;
    float old_grid_cam_center_y = grid_cam_center_y;
    zoom_grid_cam(zoom);
    grid_cam_x = old_grid_cam_center_x - ((WINDOW_WIDTH / 2) / grid_cam_zoom);
    grid_cam_y = old_grid_cam_center_y - ((WINDOW_HEIGHT / 2) / grid_cam_zoom);
}

// Player control
void rotate_player(float angle) {
    player_angle += angle;
    while (player_angle < -PI) {
        player_angle += PI * 2;
    }
    while (player_angle >= PI) {
        player_angle -= (PI * 2);
    }
}

void push_player_forward(float force) {
    if (force != 0) {
        player_x_velocity += cosf(player_angle) * force;
        player_y_velocity += sinf(player_angle) * force;
    }
}

void push_player_right(float force) {
    if (force != 0) {
        player_angle += PI_2;
        push_player_forward(force);
        player_angle -= PI_2;
    }
}

const bool *state;
void setup(void) {
    init_sound();

    // Initialize keyboard state
    state = SDL_GetKeyboardState(NULL);

    #if !__EMSCRIPTEN__
    sound_effect = MIX_LoadAudio(
        audio_mixer,
        "/Users/nallison/Documents/sfx/raycasting/Final spring flick edit 1b CLEAN FINAL2Mono.wav",
        false
    ); assert(sound_effect);
    #endif

    pixel_fov_circumference = (WINDOW_WIDTH / fov) * (PI * 2);
    radians_per_pixel = fov / WINDOW_WIDTH;
    sky_scale_x = (float) SKY_IMAGE.width / pixel_fov_circumference;
    sky_scale_y = (float) SKY_IMAGE.height / (WINDOW_HEIGHT / 2);

    precompute_shading_table();

    fp_brightness_appl = FP_BRIGHTNESS;

    // Set player start pos
    reset_player();
    reset_grid_cam();
    calc_grid_cam_center();
    enter_fps_view();
    
    mobj_create(MOBJ_NOTYPE, 983, 345, 0, 0, GRID_SPACING / 2, sprite_plant, 0);

    mobj_create(MOBJ_TESTER, 465, 217, 0, 0, GRID_SPACING / 2, sprite_boxShaded, 0);
    mobj_create(MOBJ_NOTYPE, 4 * GRID_SPACING, 3 * GRID_SPACING, GRID_SPACING / 4, 0, GRID_SPACING / 2, sprite_boxShaded, 0);
}

#define key_pressed(key) state[key]
#define key(x) SDL_SCANCODE_##x
bool prev_state[SDL_SCANCODE_COUNT];
#define key_just_pressed(scancode) (state[scancode] && !prev_state[scancode])

void process_input(void) {
    // Reset mouse pos to center if we are in mouse control cam mode
    if (mouse_focused() && !fps_free_mouse) {
        mouse_x = WINDOW_WIDTH / 2;
    }

    SDL_Event event;
    char key = 0;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_EVENT_QUIT:
                game_is_running = FALSE;
                break;
            case SDL_EVENT_MOUSE_BUTTON_UP:
                if (event.button.button == SDL_BUTTON_LEFT) left_mouse_down = FALSE;
                break;
            case SDL_EVENT_MOUSE_BUTTON_DOWN:
                if (event.button.button == SDL_BUTTON_LEFT) left_mouse_down = TRUE;
                break;
            case SDL_EVENT_MOUSE_WHEEL:
                zoom_grid_cam_center(-event.wheel.y * grid_cam_zoom_incr);
                break;
            case SDL_EVENT_MOUSE_MOTION:
                mouse_x = event.motion.x;
                mouse_y = event.motion.y;

                grid_mouse_x = grid_cam_x + (event.motion.x / grid_cam_zoom);
                grid_mouse_y = grid_cam_y + (event.motion.y / grid_cam_zoom);

                if (prev_mouse_x != -1 && prev_mouse_y != -1 && left_mouse_down) {
                    grid_cam_x -= (mouse_x - prev_mouse_x) / grid_cam_zoom;
                    grid_cam_y -= (mouse_y - prev_mouse_y) / grid_cam_zoom;
                }
                break;
            case SDL_EVENT_KEY_DOWN:
                key = event.key.key;
                break;
        }
    }

    shift = state[SDL_SCANCODE_LSHIFT] || state[SDL_SCANCODE_RSHIFT];

    if (state[SDL_SCANCODE_ESCAPE]) game_is_running = FALSE;

    if (key_just_pressed(SDL_SCANCODE_SLASH)) {
        toggle_terminal_view();
    }

    if (view == VIEW_FPS || view == VIEW_GRID) {
        if (key_just_pressed(SDL_SCANCODE_2)) enter_grid_view();
        if (key_just_pressed(SDL_SCANCODE_1)) enter_fps_view();
        if (key_just_pressed(SDL_SCANCODE_C)) toggle(&grid_follow_player);

        if (state[SDL_SCANCODE_RIGHT]) rotation_input++;
        if (state[SDL_SCANCODE_LEFT]) rotation_input--;

        if (state[SDL_SCANCODE_R]) {
            reset_grid_cam();
            reset_player();
        }

        if (key_just_pressed(key(U)) && view == VIEW_FPS) {
            fps_free_mouse = !fps_free_mouse;
            if (fps_free_mouse) {
                unfocus_mouse();
            } else {
                focus_mouse();
            }
        }

        if (state[SDL_SCANCODE_W]) vertical_input++;
        if (state[SDL_SCANCODE_S]) vertical_input--;
        if (state[SDL_SCANCODE_A]) horizontal_input--;
        if (state[SDL_SCANCODE_D]) horizontal_input++;

        if (state[SDL_SCANCODE_UP]) player_z_base++;
        if (state[SDL_SCANCODE_DOWN]) player_z_base--;

        if (player_z_base < 0) player_z_base = 0;
        else if (player_z_base > GRID_SPACING) player_z_base = GRID_SPACING;

        if (key_just_pressed(key(EQUALS))) {
            aa_level++;
        }
        if (key_just_pressed(key(MINUS)) && aa_level > 1) {
            aa_level--;
        }
        
        // Shots
        if (key_just_pressed(KEY_SHOT)) {
            shot_create(
                player_x + (cosf(player_angle + PI_2) * 10),
                player_y + (sinf(player_angle + PI_2) * 10),
                player_z - 10,
                SHOT_LENGTH,
                player_angle
            );
            sound_play_static(sound_effect, 0.25f);
        }

        // Door opening and closing
        if (key_just_pressed(KEY_OPEN_DOOR)) {
            raycast_info cast;
            raycast(player_x, player_y, player_angle, &cast, NULL, NULL);
            if (cast.door_hit.door && point_dist(player_x, player_y, cast.door_hit.x, cast.door_hit.y) <= OPEN_DOOR_DIST) {
                door *door = cast.door_hit.door;
                // If moving, reverse direction
                if (door->flags & DOORF_MOVING) {
                    door->flags ^= DOORF_OPENING;
                } else {
                    // If not moving, switch to other open/closed
                    door->flags |= DOORF_MOVING;
                    if (door->progress == 0) {
                        door->flags |= DOORF_OPENING;
                    } else {
                        door->flags &= ~DOORF_OPENING;
                    }
                }
            }
        }

    } else if (view == VIEW_TERMINAL && key != 0) {
        // If the shift key is pressed, look for a shifted character corresponding
        // with the key pressed and use if found
        // (only characters used in the font are included in BF_CHAR_KEYS)
        if (shift) {
            char new_key = BF_GetUpperCharKey(key);
            if (new_key != -1) {
                alstring_append(terminal_input, new_key);
            }
        } else if (key == ' ' || BF_GetCharIndex(key) != -1) {
            // If the key is space or in the character set, add it to the input string
            alstring_append(terminal_input, key);
        } else if (state[SDL_SCANCODE_BACKSPACE]) {
            // If backspace is pressed, delete
            alstring_pop(terminal_input);
        }

        // If return key pressed and there is text, interpret input text as command
        if (state[SDL_SCANCODE_RETURN]) {
            DT_console_text[0] = '\0';
            // If command was not found, print error message
            if (!DT_InterpretCommand(terminal, terminal_input->text)) {
                DT_ConsolePrintln("Err no such command");
            }
            alstring_clear(terminal_input);
        }
    }

    memcpy(prev_state, state, sizeof(prev_state));
}

void update(void) {
    int time_to_wait = FRAME_TARGET_TIME - (SDL_GetTicks() - last_frame_time);

    // Only delay if we are too fast to update this frame
    if (cap_fps && time_to_wait > 0 && time_to_wait <= FRAME_TARGET_TIME) {
        SDL_Delay(time_to_wait);
    }
    // Get a delta time factor converted to seconds to be used to update my objects
    delta_time = (SDL_GetTicks() - last_frame_time) / 1000.0f;

    last_frame_time = SDL_GetTicks();


    // Perform mobj behavior
    mobj *m_next;
    for (mobj *o = mobj_head; o; o = m_next) {
        m_next = o->next;
        if (mobj_behaviors[o->type]) {
            mobj_behaviors[o->type](o);
        }
    }

    // Update doors
    for (int i = 0; i < NUM_DOORS; i++) {
        door *d = doors + i;
        if (d->flags & DOORF_MOVING) {
            d->progress += (d->flags & DOORF_OPENING ? DOOR_SPEED : -DOOR_SPEED) * delta_time;
            
            // Bounds check
            if (d->flags & DOORF_OPENING && d->progress >= GRID_SPACING) {
                d->progress = GRID_SPACING;
                d->flags &= ~DOORF_MOVING;
            } else if (d->flags | ~DOORF_MOVING && d->progress <= 0) {
                d->progress = 0;
                d->flags &= ~DOORF_MOVING;
            }
        }
    }

    push_player_forward(player_movement_accel * vertical_input * delta_time);
    push_player_right(player_movement_accel * horizontal_input * delta_time);

    if (view == VIEW_FPS && !fps_free_mouse) {
        rotate_player((mouse_x - (WINDOW_WIDTH / 2)) * player_sensitivity * delta_time);
        // Reset mouse to center of screen
        SDL_WarpMouseInWindow(window, WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2);
    } else {
        rotate_player(rotation_input * player_rotation_speed * delta_time);
    }


    // Calculate velocity angle
    float player_velocity_angle = atanf(player_y_velocity / player_x_velocity);
    float player_velocity = sqrtf(powf(player_x_velocity, 2) + powf(player_y_velocity, 2));
    if (player_x_velocity < 0) player_velocity_angle += M_PI;

    // Cap player velocity within range of maximum
    if (player_x_velocity != 0 || player_y_velocity != 0) {
        if (player_x_velocity == 0) {
            if (player_y_velocity > player_max_velocity)
                player_y_velocity = player_max_velocity;
            else if (player_y_velocity < -player_max_velocity)
                player_y_velocity = -player_max_velocity;

        } else if (player_y_velocity == 0) {
            if (player_x_velocity > player_max_velocity)
                player_x_velocity = player_max_velocity;
            else if (player_x_velocity < -player_max_velocity)
                player_x_velocity = -player_max_velocity;

        } else {
            if (sqrt( powf(player_x_velocity, 2) + powf(player_y_velocity, 2) ) > player_max_velocity) {
                player_x_velocity = cosf(player_velocity_angle) * player_max_velocity;
                player_y_velocity = sinf(player_velocity_angle) * player_max_velocity;
            }
        }
    }

    // Decrease player speed if not moving
    if (!horizontal_input && !vertical_input) {
        if (player_x_velocity != 0) {
            int vel_is_pos = player_x_velocity > 0;
            player_x_velocity -= cosf(player_velocity_angle) * player_movement_decel * delta_time;
            if (vel_is_pos && player_x_velocity < 0)
                player_x_velocity = 0;
            else if (!vel_is_pos && player_x_velocity > 0)
                player_x_velocity = 0;
        }
        if (player_y_velocity != 0) {
            int vel_is_pos = player_y_velocity > 0;
            player_y_velocity -= sinf(player_velocity_angle) * player_movement_decel * delta_time;
            if (vel_is_pos && player_y_velocity < 0)
                player_y_velocity = 0;
            else if (!vel_is_pos && player_y_velocity > 0)
                player_y_velocity = 0;
        }
    }

    // Wall collisions
    int northeast_collision = get_map_coords(player_x + player_radius, player_y - player_radius);
    int southeast_collision = get_map_coords(player_x + player_radius, player_y + player_radius);
    int southwest_collision = get_map_coords(player_x - player_radius, player_y + player_radius);
    int northwest_collision = get_map_coords(player_x - player_radius, player_y - player_radius);

    #define push_left() player_x = ((((int) player_x + player_radius) / GRID_SPACING) * GRID_SPACING) - player_radius/*; printf("prev_player_x - player_x = %f\n", prev_player_x - player_x); player_y_velocity -= (prev_player_x - player_x) / tanf(player_velocity_angle)*/
    #define push_down() player_y = (((((int) player_y - player_radius) / GRID_SPACING) + 1) * GRID_SPACING) + player_radius
    #define push_right() player_x = (((((int) player_x - player_radius) / GRID_SPACING) + 1) * GRID_SPACING) + player_radius
    #define push_up() player_y = ((((int) player_y + player_radius) / GRID_SPACING) * GRID_SPACING) - player_radius

    #define push_left_wall() push_left(); player_x_velocity = 0
    #define push_down_wall() push_down(); player_y_velocity = 0
    #define push_right_wall() push_right(); player_x_velocity = 0
    #define push_up_wall() push_up(); player_y_velocity = 0

    if (northeast_collision) {
        if (southeast_collision || northwest_collision) {
            if (southeast_collision && player_x_velocity >= 0) {
                push_left_wall();
            }
            if (northwest_collision) {
                push_down_wall();
            }
        } else {
            int overlapX = GRID_SPACING - (((int) player_x + player_radius) % GRID_SPACING);
            int overlapY = ((int) player_y - player_radius) % GRID_SPACING;
            if (overlapX > overlapY && player_x_velocity >= 0) {
                push_left_wall();
            } else {
                push_down_wall();
            }
        }
    } else if (southwest_collision) {
        if (northwest_collision || southeast_collision) {
            if (northwest_collision) {
                push_right_wall();
            }
            if (southeast_collision && player_y_velocity >= 0) {
                push_up_wall();
            }
        } else {
            int overlapX = GRID_SPACING - (((int) player_x - player_radius) % GRID_SPACING);
            int overlapY = ((int) player_y + player_radius) % GRID_SPACING;
            if (overlapX < overlapY) {
                push_right_wall();
            } else if (player_y_velocity >= 0) {
                push_up_wall();
            }
        }
    } else if (northwest_collision) {
        if (northeast_collision || southwest_collision) {
            if (northeast_collision) {
                push_down_wall();
            }
            if (southwest_collision) {
                push_right_wall();
            }
        } else {
            int overlapX = GRID_SPACING - (((int) player_x - player_radius) % GRID_SPACING);
            int overlapY = GRID_SPACING - (((int) player_y - player_radius) % GRID_SPACING);
            if (overlapX < overlapY) {
                push_right_wall();
            } else {
                push_down_wall();
            }
        }
    }
    if (southeast_collision) {
        if (southwest_collision || northeast_collision) {
            if (southwest_collision && player_y_velocity >= 0) {
                push_up_wall();
            }
            if (northeast_collision && player_x_velocity >= 0) {
                push_left_wall();
            }
        } else { 
            int overlapX = ((int) player_x + player_radius) % GRID_SPACING;
            int overlapY = ((int) player_y + player_radius) % GRID_SPACING;
            if (overlapX < overlapY && player_x_velocity >= 0) {
                push_left_wall();
            } else if (player_y_velocity >= 0) {
                push_up_wall();
            }
        }
    }

    // Move player by velocity
    player_x += player_x_velocity * delta_time;
    player_y += player_y_velocity * delta_time;

    // Move shots
    shot *next;
    for (shot *s = shot_head; s; s = next) {
        next = s->next;
        shot_advance(s);
    }

    // Count shots
    /* int shot_count = 0;
    for (shot *s = shot_head; s; s = s->next) {
        shot_count++;
    }
    debug_print(shot_count, "%d"); */

    if (show_player_trail) {
        fill_dgp(player_x, player_y, DG_YELLOW);
    }

    if (grid_follow_player) {
        grid_cam_x = player_x - ((WINDOW_WIDTH / 2) / grid_cam_zoom);
        grid_cam_y = player_y - ((WINDOW_HEIGHT / 2) / grid_cam_zoom);
    }

    // Rotate all objects
    /* for (mobj *o = mobj_head; o; o = o->next) {
        o->angle += PI / 90;
        if (o->angle > PI * 2) {
            o->angle -= PI * 2;
        }
    } */

    sound_clear_finished();

    // Update positional sounds
    #if !__EMSCRIPTEN__
    if (frames % 5 == 0) {
        for (pos_sound *s = pos_sound_head; s; s = s->next) {
            sound_update_pos_pan(s);
        }
    }
    #endif

    // Update movement for weapon graphic
    weapon_cycle_progress += player_velocity * WEAPON_GRAPHIC_CYCLE_SPEED_FACTOR * delta_time;
    if (weapon_cycle_progress >= WEAPON_GRAPHIC_CYCLE_LENGTH) {
        weapon_cycle_progress -= WEAPON_GRAPHIC_CYCLE_LENGTH;
    }

    vertical_input = 0;
    horizontal_input = 0;
    rotation_input = 0;
    prev_mouse_x = mouse_x;
    prev_mouse_y = mouse_y;
    frames++;
}

void render(void) {
    set_draw_color_rgb(C_BLACK);
    SDL_RenderClear(renderer);
    clear_array_window(0);

    if ((view == VIEW_FPS && fp_show_walls) || show_player_vision || grid_casting) { // Raycasting
        float ray_dists[WINDOW_WIDTH];
        float *floor_side_dists = NULL;
        int floor_side_dists_len = 0;

        // Update weapon with walk movement cycle
        float weapon_graphic_frames_radians = weapon_cycle_progress * ((float) (PI * 4) / WEAPON_GRAPHIC_CYCLE_LENGTH);
        int weapon_graphic_x = roundf(cosf(weapon_graphic_frames_radians + (float) PI_2) * WEAPON_GRAPHIC_CYCLE_X_DISP);
        int weapon_graphic_y = roundf(sinf(weapon_graphic_frames_radians * 2) * WEAPON_GRAPHIC_CYCLE_Y_DISP);

        // Update player height bob
        player_z = player_z_base + (sinf(weapon_graphic_frames_radians * PLAYER_Z_BOB_SPEED_FACTOR) * PLAYER_Z_BOB_DISP);
        if (player_z < 0) {
            player_z = 0;
        } else if (player_z > GRID_SPACING) {
            player_z = GRID_SPACING;
        }

        // Offset by half a radian pixel so that raycast goes up the center of the pixel col
        float relative_ray_angle = (-fov + radians_per_pixel) / 2;
        int sky_start = -1;

        // Raycast, draw walls, draw floors
        for (int ray_i = 0; ray_i < WINDOW_WIDTH; ray_i++) {
            // Calculate ray angle
            relative_ray_angle += radians_per_pixel;
            float ray_angle = player_angle + relative_ray_angle;
            fix_angle_180(ray_angle);

            if (sky_start == -1) {
                sky_start = ray_angle * (pixel_fov_circumference / (PI * 2));
            }

            #if FLOOR_TEXTURES
            float rel_cos = cosf(relative_ray_angle);
            float angle_cos = cosf(ray_angle);
            float angle_sin = sinf(ray_angle);
            #endif

            int wall_texture_index, wall_texture_x;
            raycast_info cast;
            xy hit = raycast(player_x, player_y, ray_angle, &cast, &wall_texture_index, &wall_texture_x);

            float aa_worlspace_incr = WORLDSPACE / (aa_level + 1);

            if ((view == VIEW_FPS && fp_show_walls) || grid_casting) {
                // Get hit distance
                ray_dists[ray_i] = fp_dist(hit.x, hit.y, ray_angle);

                if (view == VIEW_FPS && fp_show_walls) {
                    float wall_height = project(ray_dists[ray_i]);
                    float offset_y = player_height_y_offset(wall_height);

                    // Draw texture column offset based on player height
                    char is_wall_visible = ray_dists[ray_i] <= FP_RENDER_DISTANCE;
                    float start_wall = ((WINDOW_HEIGHT - wall_height) / 2.0f) + offset_y;

                    if (is_wall_visible) {
                        #if WALL_TEXTURES
                        float texture_row_height = wall_height / TEXTURE_WIDTH;
                        
                        float row_y = start_wall;
                        #if SLOW_LIGHTING
                        #define OFF 0.01f
                        float x_off = cast.quadrant == 1 || cast.quadrant == 4 ? -OFF : OFF;
                        float y_off = cast.quadrant == 1 || cast.quadrant == 2 ? -OFF : OFF;
                        float dist = point_dist(hit.x, hit.y, mobj_head->x, mobj_head->y) * 2;
                        char check = dist < FP_RENDER_DISTANCE && raycast_to(hit.x + x_off, hit.y + y_off, mobj_head->x, mobj_head->y);
                        for (int texture_y = 0; texture_y < TEXTURE_WIDTH; texture_y++) {
                            rgb original_color = textures[wall_texture_index][texture_y][wall_texture_x];
                            //draw_col_frgb(ray_i, row_y, texture_row_height, shade_rgb(original_color, ray_dists[ray_i]));
                            if (check) {
                                draw_col_frgb(ray_i, row_y, texture_row_height, shade_rgb(original_color, dist));
                            } else {
                                draw_col_frgb(ray_i, row_y, texture_row_height, shade_rgb(original_color, FP_RENDER_DISTANCE * 0.9f));
                            }

                            row_y += texture_row_height;
                        }
                        #else
                        for (int texture_y = 0; texture_y < TEXTURE_WIDTH; texture_y++) {
                            rgb original_color = textures[wall_texture_index][texture_y][wall_texture_x];
                            draw_col_frgb(ray_i, row_y, texture_row_height, shade_rgb(original_color, ray_dists[ray_i]));

                            row_y += texture_row_height;
                        }
                        #endif
                        #else
                        draw_col_rgb(ray_i, start_wall, wall_height, shade_rgb(C_WHITE, ray_dists[ray_i]));
                        #endif
                    }
                    
                    // Draw ceiling and floor
                    // Iterate enough to draw the entire ceiling or floor side, whichever is taller
                    #if FLOOR_TEXTURES
                    int end_ceiling = roundf(start_wall);
                    int end_floor = roundf(WINDOW_HEIGHT - (start_wall + wall_height));
                    int end_draw = end_floor > end_ceiling ? end_floor : end_ceiling;

                    float ceil_dist_factor = (GRID_SPACING - player_z) / player_z;
                    float pixel_y_worldspace = WORLDSPACE / 2;
                    for (int pixel_y = 0; pixel_y < end_draw; pixel_y++) {
                        // Optimization possible:
                        // Calculate for floor or height distance depending on which will be calculated the most
                        // number of times so that conversion isn't necessary when only the side that needs conversion
                        // is being calculated.
                        Uint16 r_sum_floor = 0;
                        Uint16 g_sum_floor = 0;
                        Uint16 b_sum_floor = 0;
                        Uint16 r_sum_ceil = 0;
                        Uint16 g_sum_ceil = 0;
                        Uint16 b_sum_ceil = 0;
                        char calc_dist = pixel_y == (floor_side_dists_len / aa_level);

                        // Perform anti-aliasing: get average of colors found in seperate places in the same pixel
                        for (int i = 0; i < aa_level; i++) {
                            if (calc_dist) {
                                // If we have not calculated this distance yet, do so and store
                                floor_side_dists = realloc(floor_side_dists, ++floor_side_dists_len * sizeof(float));
                                floor_side_dists[pixel_y + i] = (player_z / ((GRID_SPACING / 2) - (pixel_y_worldspace + (i * aa_worlspace_incr)))) * fp_scale;
                            }
                            float floor_side_dist = floor_side_dists[pixel_y + i];

                            if (pixel_y < end_floor) {
                                float straight_dist = floor_side_dist / rel_cos;
                                
                                float point_x = (angle_cos * straight_dist) + player_x;
                                float point_y = (angle_sin * straight_dist) + player_y;

                                Uint8 texture_num = floor_textures[(int) point_y / GRID_SPACING][(int) point_x / GRID_SPACING];
                                
                                // If we are drawing the sky texture
                                if (texture_num == 0) {
                                    int sky_x = sky_start + ray_i;
                                    if (sky_x >= pixel_fov_circumference) {
                                        sky_x -= pixel_fov_circumference;
                                    }

                                    rgba color = get_graphics(SKY_IMAGE, (int) (sky_x * sky_scale_x), (int) (pixel_y * sky_scale_y));
                                    //set_pixel_rgba(ray_i, WINDOW_HEIGHT - pixel_y - 1, color);
                                    r_sum_floor += color.r;
                                    g_sum_floor += color.g;
                                    b_sum_floor += color.b;
                                
                                // If we are drawing a floor texture
                                } else {
                                    int texture_x = fmodf(point_x, GRID_SPACING) * ((float) TEXTURE_WIDTH / GRID_SPACING);
                                    int texture_y = fmodf(point_y, GRID_SPACING) * ((float) TEXTURE_WIDTH / GRID_SPACING);

                                    // Offset texture num by one because texture zero is the sky texture num
                                    #if !SLOW_LIGHTING
                                    rgb color = shade_rgb(textures[texture_num - 1][texture_y][texture_x], floor_side_dist);
                                    
                                    //set_pixel_rgb(ray_i, WINDOW_HEIGHT - pixel_y - 1, color);
                                    r_sum_floor += color.r;
                                    g_sum_floor += color.g;
                                    b_sum_floor += color.b;
                                    #else
                                    rgb original_color = textures[texture_num - 1][texture_y][texture_x];
                                    float light_dist = point_dist(point_x, point_y, mobj_head->x, mobj_head->y) * 2;
                                    if (light_dist < FP_RENDER_DISTANCE && raycast_to(point_x, point_y, mobj_head->x, mobj_head->y)) {
                                        set_pixel_rgb(ray_i, WINDOW_HEIGHT - pixel_y - 1, shade_rgb(original_color, light_dist));
                                    } else {
                                        set_pixel_rgb(ray_i, WINDOW_HEIGHT - pixel_y - 1, shade_rgb(original_color, FP_RENDER_DISTANCE * 0.9f));
                                    }
                                    #endif
                                }
                            }

                            if (pixel_y < end_ceiling) {
                                float ceil_side_dist = floor_side_dist * ceil_dist_factor;
                                float straight_dist = ceil_side_dist / rel_cos;

                                float point_x = (angle_cos * straight_dist) + player_x;
                                float point_y = (angle_sin * straight_dist) + player_y;

                                Uint8 texture_num = ceiling_textures[(int) point_y / GRID_SPACING][(int) point_x / GRID_SPACING];
                                
                                // If we are drawing the sky texture
                                if (texture_num == 0) {
                                    int sky_x = sky_start + ray_i;
                                    if (sky_x >= pixel_fov_circumference) {
                                        sky_x -= pixel_fov_circumference;
                                    }

                                    rgba color = get_graphics(SKY_IMAGE, (int) (sky_x * sky_scale_x), (int) (pixel_y * sky_scale_y));
                                    r_sum_ceil += color.r;
                                    g_sum_ceil += color.g;
                                    b_sum_ceil += color.b;
                                
                                // If we are drawing a ceiling texture
                                } else {
                                    int texture_x = fmodf(point_x, GRID_SPACING) * ((float) TEXTURE_WIDTH / GRID_SPACING);
                                    int texture_y = fmodf(point_y, GRID_SPACING) * ((float) TEXTURE_WIDTH / GRID_SPACING);

                                    // Offset texture num by one because texture zero is the sky texture num
                                    rgb color = shade_rgb(textures[texture_num - 1][texture_y][texture_x], ceil_side_dist);
                                    r_sum_ceil += color.r;
                                    g_sum_ceil += color.g;
                                    b_sum_ceil += color.b;
                                }
                            }
                        }

                        // Take average of colors
                        if (pixel_y < end_floor) {
                            set_pixel_rgb(
                                ray_i, WINDOW_HEIGHT - pixel_y - 1, 
                                (rgb) {
                                    r_sum_floor / aa_level,
                                    g_sum_floor / aa_level,
                                    b_sum_floor / aa_level
                                }
                            );
                        }
                        if (pixel_y < end_ceiling) {
                            set_pixel_rgb(
                                ray_i, pixel_y,
                                (rgb) {
                                    r_sum_ceil / aa_level,
                                    g_sum_ceil / aa_level,
                                    b_sum_ceil / aa_level
                                }
                            );
                        }
                        #endif

                        pixel_y_worldspace += WORLDSPACE;
                    }
                    //#endif
                }
            }
        }

        //printf("%llu\n", SDL_GetTicks() - millis);

        if (floor_side_dists_len != 0) {
            free(floor_side_dists);
            floor_side_dists_len = 0;
        }

        #if SPRITES
        if (view == VIEW_FPS || grid_casting) {
            // Store positions and distances of sprites on screen sorted by distance
            for (mobj *o = mobj_head; o; o = o->next) {
                add_sprite_proj_mobj(o);
            }

            // Give sprites to shots
            for (shot *s = shot_head; s; s = s->next) {
                float x = s->x1;
                float y = s->y1;
                for (int i = 0; i < SHOT_SPRITE_COUNT; i++) {
                    add_sprite_proj(x, y, s->z, sprite_shot);
                    x += s->x_spr_incr;
                    y += s->y_spr_incr;
                }
            }

            // Render sprites
            for (sprite_proj *proj = sprite_proj_head; proj; proj = proj->next) {
                // Calculate screen x pos
                float center_x = angle_to_screen_x(proj->angle);

                sprite *spr = sprites + proj->sprite_num;

                float proj_height = project(proj->dist);
                float sprite_height = proj_height * spr->world_height_percent;
                float sprite_width = (proj_height / spr->height) * spr->world_height_percent * spr->width;

                // Don't draw if the sprite is completely offscreen
                if (center_x > WINDOW_WIDTH + (sprite_width / 2) && center_x < pixel_fov_circumference - (sprite_width / 2)) {
                    continue;
                }

                // Calculate bounds and increments for rendering
                float start_x = center_x - (sprite_width / 2);
                if (start_x > pixel_fov_circumference - sprite_width) {
                    start_x -= pixel_fov_circumference;
                }

                // Limit x to inside of screen
                int end_x = min(roundf(start_x + sprite_width), WINDOW_WIDTH);
                float skipped_x = 0;
                if (start_x < 0) {
                    skipped_x = -start_x;
                    start_x = 0;
                }

                float start_y_f =
                    ((WINDOW_HEIGHT + proj_height) / 2) - // Y coord of the floor at the distance
                    ((proj->z / GRID_SPACING) * proj_height) -  // Height pos of object
                    (spr->origin_y_offset_percent * sprite_height) - // Sprite origin offset
                    sprite_height +  // Height of sprite graphics (drawing downward from top of screen)
                    player_height_y_offset(proj_height); // Offset perspective from player height
                
                int start_y = roundf(start_y_f);
                int end_y = roundf(start_y_f + sprite_height);

                // Increments for source spr read position
                float image_y_incr = (float) spr->height / (end_y - start_y);
                float image_x_incr = spr->width / sprite_width;

                // Limit to inside of the screen
                float skipped_y = 0;
                if (start_y < 0) {
                    skipped_y = -start_y_f;
                    start_y = 0;
                }
                end_y = min(end_y, WINDOW_HEIGHT);

                // Render by column
                float image_x = skipped_x * image_x_incr;
                float start_image_y = skipped_y * image_y_incr;
                for (int render_x = roundf(start_x); render_x < end_x; render_x++) {
                    // Hide behind walls
                    if (proj->dist < ray_dists[render_x]) {
                        float image_y = start_image_y;
                        for (int render_y = start_y; render_y < end_y; render_y++) {
                            rgba c = spr->pixels[(spr->width * (int) image_y) + (int) image_x];
                            set_pixel_rgba(render_x, render_y, shade_rgba(c, proj->dist));
                            image_y += image_y_incr;
                        }
                    }
                    image_x += image_x_incr;
                }
            }

            sprite_proj_destroy_all();
        }
        #endif
        
        // Weapon graphic
        if (fp_show_weapon && view == VIEW_FPS) {
            #if __EMSCRIPTEN__
            #define WEAPON_SCALE 1
            #define WEAPON_POS_OFFSET_X 1.5f
            #define WEAPON_POS_OFFSET_Y 3
            #else
            #define WEAPON_SCALE 3
            #define WEAPON_POS_OFFSET_X 2
            #define WEAPON_POS_OFFSET_Y 2
            #endif
            draw_image_scale(
                image_weapon2ForGame,
                (WINDOW_WIDTH - ((79 * WEAPON_SCALE) * WEAPON_POS_OFFSET_X)) + weapon_graphic_x,
                (WINDOW_HEIGHT - ((142 * WEAPON_SCALE) / WEAPON_POS_OFFSET_Y)) - weapon_graphic_y,
                WEAPON_SCALE
            );
        }
    }

    if (view == VIEW_GRID) { // Grid rendering
        // Grid box fill
        for (int row = 0; row < GRID_HEIGHT; row++) {
            for (int col = 0; col < GRID_WIDTH; col++) {
                Uint8 space = get_map(col, row);
                switch (space) {
                    case TILE_NONSOLID:
                    case TILE_SOLID:
                        g_draw_rect_rgb(
                            col * GRID_SPACING, row * GRID_SPACING, GRID_SPACING, GRID_SPACING,
                            space == TILE_NONSOLID ? grid_fill_nonsolid : grid_fill_solid
                        );
                        break;
                    case TILE_HORIZ_DOOR:
                        //g_draw_rect_rgb(col * GRID_SPACING, row * GRID_SPACING, GRID_SPACING, GID)
                        break;
                }
                switch (space) {
                    case TILE_NONSOLID:
                    case TILE_HORIZ_DOOR:
                        g_draw_rect_rgb(col * GRID_SPACING, row * GRID_SPACING, GRID_SPACING, GRID_SPACING, grid_fill_nonsolid);
                        if (space == TILE_HORIZ_DOOR) {
                            door *door = get_door(col, row);
                            float progress;
                            if (door) {
                                progress = door->progress;
                            } else {
                                progress = GRID_SPACING;
                            }

                            g_draw_rect_rgb(
                                col * GRID_SPACING, (row * GRID_SPACING) + (GRID_SPACING / 2) - (GRID_DOOR_THICKNESS / 2),
                                progress, GRID_DOOR_THICKNESS, grid_fill_solid
                            );
                        }
                        break;
                    case TILE_SOLID:
                        g_draw_rect_rgb(col * GRID_SPACING, row * GRID_SPACING, GRID_SPACING, GRID_SPACING, grid_fill_solid);
                        break;
                }
            }
        }

        if (show_grid_lines) { // Grid lines
            // Vertical
            for (int i = 0; i < GRID_WIDTH + 1; i++) {
                g_draw_rect_rgb(
                    (i * GRID_SPACING) - (grid_line_width / 2.0f), 
                    0,
                    grid_line_width,
                    GRID_HEIGHT * GRID_SPACING,
                    grid_line_fill
                );
            }
            // Horizontal
            for (int i = 0; i < GRID_WIDTH + 1; i++) {
                g_draw_rect_rgb(
                    0,
                    (i * GRID_SPACING) - (grid_line_width / 2.0f),
                    GRID_WIDTH * GRID_SPACING,
                    grid_line_width,
                    grid_line_fill
                );
            }
        }

        // Map objects
        for (mobj *temp = mobj_head; temp; temp = temp->next) {
            g_draw_scale_point_rgb(temp->x, temp->y, temp->radius, grid_mobj_color);
            g_draw_scale_point_rgb(
                temp->x + (cosf(temp->angle) * temp->radius),
                temp->y + (sinf(temp->angle) * temp->radius),
                (temp->radius / 3) + 1,
                grid_mobj_color
            );
        }

        // Shots
        for (shot *s = shot_head; s; s = s->next) {
            g_draw_scale_point_rgb(s->x1, s->y1, 2.5f, C_ORANGE);
            g_draw_scale_point_rgb(s->x2, s->y2, 3, C_ORANGE);
        }

        // Fill debug grid points
        for (struct fill_dgp *p_i = fill_dgp_head; p_i; p_i = p_i->next) {
            g_draw_point_rgb(p_i->x, p_i->y, dgp_radius, DG_COLORS[p_i->color_index]);
        }

        // Player direction pointer
        g_draw_scale_point_rgb(
            player_x + cosf(player_angle) * grid_player_pointer_dist,
            player_y + sinf(player_angle) * grid_player_pointer_dist,
            player_radius / 2.0f,
            grid_player_fill
        );
        // Player
        g_draw_scale_point_rgb(player_x, player_y, player_radius, grid_player_fill);

        // Temporary debug grid points
        for (size_t i = 0; i < num_temp_dgps; i++) {
            g_draw_point_rgb(temp_dgp_list[i].x, temp_dgp_list[i].y, 
                dgp_radius * (temp_dgp_list[i].color_index == DG_BLUE ? 1.5f : 1),
                DG_COLORS[temp_dgp_list[i].color_index]);
        }

        // Grid crosshair
        if (show_grid_crosshairs) {
            draw_rect_frgb(WINDOW_WIDTH / 2, 0, 1, WINDOW_HEIGHT, C_WHITE);
            draw_rect_frgb(0, WINDOW_HEIGHT / 2, WINDOW_WIDTH, 1, C_WHITE);
        }

        if (show_mouse_coords) {
            // Test raycast_to
            raycast_info v;
            float angle = get_angle_from_player(grid_mouse_x, grid_mouse_y);
            ray_hit result = raycast_to_x(player_x, player_y, grid_mouse_x, angle, &v);

            if (result == RAY_NOHIT) {
                draw_point_frgb(mouse_x, mouse_y, 5, C_ORANGE);
            } else if (result == RAY_HORIZHIT) {
                g_draw_point_rgb(v.c_hx, v.c_hy * GRID_SPACING, 5, C_RED);
            } else {
                g_draw_point_rgb(v.c_vx * GRID_SPACING, v.c_vy, 5, C_PURPLE);
            }

            // On-screen mouse coords
            char *coords_text;
            asprintf(&coords_text, "(%d, %d) %s", (int) grid_mouse_x, (int) grid_mouse_y, get_map_coords(grid_mouse_x, grid_mouse_y) ? "true" : "false");
            BF_DrawTextRgb(coords_text, mouse_x, mouse_y, 3, -1, C_RED, FALSE);
            free(coords_text);
        }
    } else if (view == VIEW_TERMINAL) { // Terminal view
        // Output from previous command
        BF_DrawTextRgb(DT_console_text, 0, 0, terminal_font_size, WINDOW_WIDTH, terminal_font_color, FALSE);
    
        // Prompt string
        BF_FillTextRgb(TERMINAL_PROMPT, terminal_font_size, WINDOW_WIDTH, terminal_font_color, FALSE);

        // Input text
        BF_FillTextRgb(terminal_input->text, terminal_font_size, WINDOW_WIDTH, terminal_font_color, TRUE);
    }

    // FPS readout
    if (show_fps && view != VIEW_TERMINAL) {
        #define FPS_READOUT_SIZE 5
        char *fps_text;
        asprintf(&fps_text, "fps %f", 1 / delta_time);
        BF_DrawText(fps_text, 0, 0, FPS_READOUT_SIZE, -1, 255, 255, 255, FALSE);
        free(fps_text);
    }

    // Clear dg things
    num_temp_dgps = 0;
    num_dgls = 0;

    #if APPLY_BLUR
    for (int row = 0; row < WINDOW_HEIGHT; row += 2) {
        for (int col = 0; col < WINDOW_WIDTH; col += 2) {
            rgba avg = {
                (pixel_array[row][col].r + pixel_array[row + 1][col].r + pixel_array[row][col + 1].r + pixel_array[row + 1][col + 1].r) / 4.0f,
                (pixel_array[row][col].g + pixel_array[row + 1][col].g + pixel_array[row][col + 1].g + pixel_array[row + 1][col + 1].g) / 4.0f,
                (pixel_array[row][col].b + pixel_array[row + 1][col].b + pixel_array[row][col + 1].b + pixel_array[row + 1][col + 1].b) / 4.0f,
                255
            };
            pixel_array[row][col] = avg;
            pixel_array[row + 1][col] = avg;
            pixel_array[row][col + 1] = avg;
            pixel_array[row + 1][col + 1] = avg;
        }
    }
    #endif

    present_array_window();
    SDL_RenderPresent(renderer);
}

void free_memory(void) {
    // Fill dgps
    struct fill_dgp *p_i = fill_dgp_head;
    while (p_i) {
        struct fill_dgp *next = p_i->next;
        free(p_i);
        p_i = next;
    }

    alstring_destroy(terminal_input);
    mobj_destroy_all();
    shot_destroy_all();
}

// Web integration setup
#if __EMSCRIPTEN__
void main_loop(void) {
    process_input();
    update();
    render();
}

int main() {
    if (!initialize_window(SDL_INIT_VIDEO, "Raycasting on the web")) {
        return 1;
    }

    initialize_array_window();
    setup();
    init_debugging();

    emscripten_set_main_loop(main_loop, 0, 1);

    debugging_end();
    destroy_window();
    destroy_array_window();
    free_memory();

    return 0;
}
#else
int main() {
    printf("Start\n");

    game_is_running = initialize_window(SDL_INIT_VIDEO | SDL_INIT_AUDIO, "Raycasting");
    initialize_array_window();

    setup();
    init_debugging();

    while (game_is_running) {
        process_input();
        update();
        render();
    }

    debugging_end();
    destroy_window();
    destroy_array_window();
    free_memory();

    return 0;
}
#endif