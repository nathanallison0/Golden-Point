#ifndef _GPOINT_ENGINE_H_
#define _GPOINT_ENGINE_H_

#include <SDL3Start.h>
#include <mobj.h>

extern Uint32 frames;
extern Uint8 cap_fps;
extern float delta_time;
extern Uint8 show_mouse_coords;
extern mobj *player;
extern Uint8 idle;
extern int player_health;
extern Uint8 tester;
extern Uint8 fp_show_weapon;
extern Uint8 do_enemies;
extern Uint8 defeated_enemies;
extern Uint8 show_crosshair;
extern Uint8 show_grid_lines;
extern Uint8 fp_show_walls;
extern Uint8 show_fps;
extern Uint8 aa_level;
extern Uint8 hide_mouse;
extern float player_sensitivity;
extern float enemy_range;
extern float sky_off;

float point_dist(float x1, float y1, float x2, float y2);

float get_angle_to(float x1, float y1, float x2, float y2);

float get_angle_from_player(float x, float y);

float get_angle_to_player(float x, float y);

void reset(void);

#endif