#ifndef _GPOINT_BEHAVIOR_H_
#define _GPOINT_BEHAVIOR_H_

#include <SDL3Start.h>
#include <mobj.h>
#include <extra.h>

typedef struct mobj mobj;
typedef Uint8 mobj_type;

#define ENEMY_DEFEATED_SPRITE sprite_plant

enum {
    SES_WATCH,
    SES_MIGRATE,
    SES_CHECK,
    SES_CONFIRM,
    SES_ATTACK,
    SES_FLINCH
};
typedef Uint8 se_state;

extra (
    particle,
    Uint8 frames;
    float x_vel;
    float y_vel;
    float z_vel;
);

extra (
    enemy,
    float neutral_angle;
    int health;
);

extra (
    smart_enemy,
    se_state state;
    float seconds;
    int health;
    struct {
        float x_vel;
        float y_vel;
        float *points;
        int num_points;
        int watchpoint_index;
    } route;

    struct {
        float seconds_seeing;
        float last_player_x;
        float last_player_y;
    } track;
);

typedef void (*mobj_behavior)(mobj *);

mobj_behavior get_mobj_behavior(mobj_type type);

unsigned long get_mobj_extra_size(mobj_type type);

void smart_enemy_init(void);

void smart_enemy_destroy(mobj *smart_enemy);

void smart_enemy_damage(mobj *smart_enemy, int damage);

void smart_enemy_check_coords(mobj *smart_enemy, float x, float y);

void particle_init(float x, float y, float z, Uint16 sprite_index, Uint8 dir_of_shot);

#endif