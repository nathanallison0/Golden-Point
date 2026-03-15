shot *shot_create(float, float, float, float, mobj *);

#include "pathfind.h"

bool mobj_turn_towards(mobj *o, float target_angle, float speed) {
    float angle_diff = target_angle - o->angle;
    fix_angle_180(angle_diff);

    float delta = speed * delta_time;
    
    char success;
    if (fabsf(angle_diff) <= delta) {
        o->angle = target_angle;
        success = TRUE;
    } else {
        o->angle += (angle_diff < 0 ? -delta : delta);
        success = FALSE;
    }

    fix_angle_180(o->angle);
    return success;
}

bool mobj_can_goto(mobj *o, float x, float y) {
    // Raycast from opposite corners to ensure path is open for the mobj's radius
    float angle = get_angle_to(o->x, o->y, x, y);
    float x1_off = angle > 0 ? o->radius : -o->radius;
    float y1_off = range(-PI_2, <, angle, <, PI_2) ? -o->radius : o->radius;

    float x2_off = angle > 0 ? -o->radius : o->radius;
    float y2_off = range(-PI_2, <, angle, <, PI_2) ? o->radius : -o->radius;

    raycast_info v;
    return (
        raycast_to_angle(o->x + x1_off, o->y + y1_off, x + x1_off, y + y1_off, angle, &v) == RAY_NOHIT &&
        raycast_to_angle(o->x + x2_off, o->y + y2_off, x + x2_off, y + y2_off, angle, &v) == RAY_NOHIT
    );
}

bool mobj_sees_player(mobj *o) {
    raycast_info v;
    return raycast_to_mobj(o, player, &v) == RAY_NOHIT;
}

// Particle
extra (
    particle,
    Uint8 frames;
    float x_vel;
    float y_vel;
    float z_vel;
);

#define PARTICLE_RADIUS 5
#define PARTICLE_LIFETIME 70
#define PARTICLE_POS_DISP 5
#define PARTICLE_AWAY_VEL_START (GRID_SPACING / 8)
#define PARTICLE_AWAY_VEL_DRAG 1.8f
#define PARTICLE_UP_VEL_START (GRID_SPACING / 4)
#define PARTICLE_UP_VEL_DRAG 1.005f
void particle_init(float x, float y, float z, Uint16 sprite_index, Uint8 dir_of_shot) {
    mobj *particle = mobj_create(MOBJ_PARTICLE, x, y, z, 0, PARTICLE_RADIUS, sprite_index, 0);
    
    __def_extra(particle);
    extra->frames = 0;
    
    // Set direction for particle to move in
    extra->z_vel = PARTICLE_UP_VEL_START;
    switch (dir_of_shot) {
        case 0: // east
            extra->x_vel = -PARTICLE_AWAY_VEL_START;
            extra->y_vel = 0;
            particle->x -= PARTICLE_POS_DISP;
            break;
        case 1: // north
            extra->y_vel = -PARTICLE_AWAY_VEL_START;
            extra->x_vel = 0;
            particle->y -= PARTICLE_POS_DISP;
            break;
        case 2: // west
            extra->x_vel = PARTICLE_AWAY_VEL_START;
            extra->y_vel = 0;
            particle->x += PARTICLE_POS_DISP;
            break;
        case 3: // south
            extra->y_vel = PARTICLE_AWAY_VEL_START;
            extra->x_vel = 0;
            particle->y += PARTICLE_POS_DISP;
            break;
    }
}

void particle_behave(mobj *particle) {
    __def_extra(particle);
    extra->frames++;

    if (extra->frames == PARTICLE_LIFETIME) {
        mobj_destroy(particle);
        return;
    }

    float drag_away = expf(-PARTICLE_AWAY_VEL_DRAG * delta_time);
    float drag_up = expf(-PARTICLE_UP_VEL_DRAG * delta_time);
    extra->x_vel *= drag_away;
    extra->y_vel *= drag_away;
    extra->z_vel *= drag_up;

    particle->x += extra->x_vel * delta_time;
    particle->y += extra->y_vel * delta_time;
    particle->z += extra->z_vel * delta_time;
}

// Enemy
extra (
    enemy,
    float neutral_angle;
    int health;
);

#define ENEMY_RADIUS 20 //10
#define ENEMY_HEALTH 20
#define ENEMY_START_SPRITE sprite_boxShaded
#define ENEMY_DEFEATED_SPRITE sprite_plant
void enemy_init(float x, float y, float z, float angle) {
    mobj *enemy = mobj_create(MOBJ_ENEMY, x, y, z, angle, ENEMY_RADIUS, ENEMY_START_SPRITE, MF_SHOOTABLE);
    __def_extra(enemy);
    extra->health = ENEMY_HEALTH;
    extra->neutral_angle = angle;
}

#define ENEMY_TURN_SPEED PI
#define ENEMY_SHOT_Z_OFF (GRID_SPACING / 4)
#define ENEMY_ATTACK_DELAY 30
void enemy_behave(mobj *enemy) {
    if (!do_enemies) {
        return;
    }

    __def_extra(enemy);

    // If we are defeated, don't do anything
    if (extra->health <= 0) {
        return;
    }

    // If the player is within range and can be seen, turn towards player
    // Otherwise, turn towards neutral position
    float target_angle;
    raycast_info v;
    char turning_towards_player = 
        point_dist(enemy->x, enemy->y, player->x, player->y) <= enemy_range &&
        raycast_to_mobj(enemy, player, &v) == RAY_SUCCESS;
    
    if (turning_towards_player) {
        target_angle = get_angle_to_player(enemy->x, enemy->y);
    } else {
        target_angle = extra->neutral_angle;
    }

    // If we are facing the player, attack
    if (
        mobj_turn_towards(enemy, target_angle, ENEMY_TURN_SPEED) &&
        turning_towards_player &&
        frames % ENEMY_ATTACK_DELAY == 0
    ) {
        shot_create(enemy->x, enemy->y, enemy->z + ENEMY_SHOT_Z_OFF, enemy->angle, enemy);
        sound_play_pos_mobj(sound_laser, 1.0f, enemy);
    }
}

// Smart enemy
#define SMART_ENEMY_HEALTH 50
#define SMART_ENEMY_RADIUS 20
#define SMART_ENEMY_WALK_SPEED 100
#define SMART_ENEMY_RUN_SPEED 220
#define SMART_ENEMY_WATCH_TIME 10.0f
#define SMART_ENEMY_CHECK_WAIT_TIME 2.5f
#define SMART_ENEMY_VISION_RADIUS (GRID_SPACING * 15)
#define SMART_ENEMY_ATTACK_WAIT_TIME 0.5f
#define SMART_ENEMY_SHOT_HEIGHT_OFFSET 25.6f // half height
#define SMART_ENEMY_PLAYER_SIGHT_DELAY 0.2f
#define SMART_ENEMY_FLINCH_TIME 0.2f

#define SMART_ENEMY_SPRITE_DEFAULT sprite_boxShaded
#define SMART_ENEMY_SPRITE_FLINCH sprite_boxHit
enum {
    SES_WATCH,
    SES_MIGRATE,
    SES_CHECK,
    SES_CONFIRM,
    SES_ATTACK,
    SES_FLINCH
};
typedef Uint8 se_state;

typedef struct {
    int x;
    int y;
    float angle;
    char occupied;
} se_watchpoint;

int se_occ_watchpoints = 0;
#define NUM_SE_WATCHPOINTS 7
se_watchpoint se_watchpoints[NUM_SE_WATCHPOINTS] = {
    {22, 23, PI + PI_4, 0},
    {13, 8, PI + PI_2 + PI_4, 0},
    {5, 14, PI_2 + PI_4, 0},
    {10, 23, PI, 0},
    {1, 6, PI_4, 0},
    {19, 6, PI, 0},
    {13, 10, PI_4, 0}
};

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

void smart_enemy_occupy_watchpoint(mobj *smart_enemy, int index) {
    __def_extra(smart_enemy);

    se_watchpoints[index].occupied = TRUE;
    se_occ_watchpoints++;
    extra->route.watchpoint_index = index;
}

void smart_enemy_vacate_watchpoint(mobj *smart_enemy) {
    __def_extra(smart_enemy);

    se_watchpoints[extra->route.watchpoint_index].occupied = FALSE;
    se_occ_watchpoints--;
    extra->route.watchpoint_index = -1;
}

void smart_enemy_transition(smart_enemy_extra *extra, se_state new_state) {
    // If we are following a path, free it up
    if (extra->state == SES_MIGRATE || extra->state == SES_CHECK) {
        free(extra->route.points);
    }
    
    extra->state = new_state;
}

void smart_enemy_transition_timed(mobj *smart_enemy, se_state new_state, float seconds) {
    __def_extra(smart_enemy);
    smart_enemy_transition(extra, new_state);
    extra->seconds += seconds;
}

void smart_enemy_transition_untimed(mobj *smart_enemy, se_state new_state) {
    __def_extra(smart_enemy);
    smart_enemy_transition(extra, new_state);
    extra->seconds = 0;
}

void smart_enemy_transition_jump(mobj *smart_enemy, se_state new_state, float seconds) {
    __def_extra(smart_enemy);
    smart_enemy_transition(extra, new_state);
    extra->seconds = seconds;
}

void smart_enemy_pathfind_face_point(mobj *smart_enemy, float *point) {
    __def_extra(smart_enemy);
    smart_enemy->angle = get_angle_to(smart_enemy->x, smart_enemy->y, point[0], point[1]);

    // Speed depends on what we are doing
    float speed = extra->state == SES_MIGRATE ? SMART_ENEMY_WALK_SPEED : SMART_ENEMY_RUN_SPEED;
    extra->route.x_vel = cosf(smart_enemy->angle) * speed;
    extra->route.y_vel = sinf(smart_enemy->angle) * speed;
}

void smart_enemy_pop_route_points(smart_enemy_extra *extra, int count) {
    // Remove the point by reallocating less space (point located at end of array)
    extra->route.num_points -= count;
    extra->route.points = realloc(extra->route.points, extra->route.num_points * (sizeof(float) * 2));
}

void smart_enemy_path_smooth(mobj *smart_enemy) {
    __def_extra(smart_enemy);
    
    // Remove all points where we can reach the one after unobstructed
    int points_to_remove = 0;
    for (int i = (extra->route.num_points - 2) * 2; i >= 0; i -= 2) {
        float *second_point = extra->route.points + i;
        if (!mobj_can_goto(smart_enemy, second_point[0], second_point[1])) {
            break;
        }

        points_to_remove++;
    }

    smart_enemy_pop_route_points(extra, points_to_remove);
}

void smart_enemy_goto(mobj *smart_enemy, int x, int y) {
    __def_extra(smart_enemy);

    extra->route.points = pathfind(
        (int) smart_enemy->x / GRID_SPACING, (int) smart_enemy->y / GRID_SPACING,
        x, y,
        &(extra->route.num_points)
    );

    smart_enemy_path_smooth(smart_enemy);

    // If we found a path, face the first point and set state to migrate
    if (extra->route.points) {
        float *point = extra->route.points + ((extra->route.num_points - 1) * 2);
        smart_enemy_pathfind_face_point(smart_enemy, point);
    }
}

void smart_enemy_goto_watchpoint(mobj *smart_enemy) {
    __def_extra(smart_enemy);
    smart_enemy_goto(
        smart_enemy,
        se_watchpoints[extra->route.watchpoint_index].x,
        se_watchpoints[extra->route.watchpoint_index].y
    );
}

void smart_enemy_goto_coords(mobj *smart_enemy, float x, float y) {
    smart_enemy_goto(smart_enemy, (int) x / GRID_SPACING, (int) y / GRID_SPACING);
}

void smart_enemy_check_coords(mobj *smart_enemy, float x, float y) {
    smart_enemy_transition_untimed(smart_enemy, SES_CHECK);
    smart_enemy_goto_coords(smart_enemy, x, y);
}

int smart_enemy_choose_watchpoint(void) {
    if (se_occ_watchpoints == NUM_SE_WATCHPOINTS) {
        return -1;
    }

    // Choose a random point out of the remaining ones
    int vacated_index = rand() % (NUM_SE_WATCHPOINTS - se_occ_watchpoints);

    for (int i = 0, v = -1; i < NUM_SE_WATCHPOINTS; i++) {
        if (!se_watchpoints[i].occupied) {
            v++;
            if (v == vacated_index) {
                return i;
            }
        }
    }

    return -1;
}

void smart_enemy_init(void) {
    // Place enemy at first available watchpoint
    int watchpoint = smart_enemy_choose_watchpoint();

    if (watchpoint == -1) {
        printf("could not create smart enemy: no available watchpoints\n");
        return;
    }

    mobj *smart_enemy = mobj_create(
        MOBJ_SMART_ENEMY,
        (se_watchpoints[watchpoint].x * GRID_SPACING) + GRID_SPACING_2,
        (se_watchpoints[watchpoint].y * GRID_SPACING) + GRID_SPACING_2,
        GRID_SPACING / 4, // z
        se_watchpoints[watchpoint].angle,
        SMART_ENEMY_RADIUS,
        SMART_ENEMY_SPRITE_DEFAULT,
        MF_SHOOTABLE
    );

    smart_enemy_occupy_watchpoint(smart_enemy, watchpoint);

    __def_extra(smart_enemy);
    extra->health = SMART_ENEMY_HEALTH;
    extra->route.num_points = 0;
    extra->state = SES_WATCH;
    extra->seconds = SMART_ENEMY_WATCH_TIME;
    extra->track.seconds_seeing = 0;
}

void smart_enemy_track_player(mobj *smart_enemy) {
    __def_extra(smart_enemy);
    extra->track.last_player_x = player->x;
    extra->track.last_player_y = player->y;
    smart_enemy->angle = get_angle_to_player(smart_enemy->x, smart_enemy->y);
}

void smart_enemy_behave(mobj *smart_enemy) {
    if (!do_enemies) {
        return;
    }

    __def_extra(smart_enemy);

    // If we are not attacking and we can see the player,
    // add to seeing counter
    raycast_info v;
    if (
        extra->state != SES_ATTACK && extra->state != SES_FLINCH &&
        point_dist(smart_enemy->x, smart_enemy->y, player->x, player->y) <= SMART_ENEMY_VISION_RADIUS &&
        raycast_to_mobj(smart_enemy, player, &v) == RAY_NOHIT
    ) {
        extra->track.seconds_seeing += delta_time;

        // If we have been seeeing the player for long enough, attack
        if (extra->track.seconds_seeing >= SMART_ENEMY_PLAYER_SIGHT_DELAY) {
            extra->track.seconds_seeing = 0;

            // If we are at a watchpoint, vacate
            if (extra->route.watchpoint_index != -1) {
                smart_enemy_vacate_watchpoint(smart_enemy);
            }

            // Transition to attack state
            smart_enemy_transition_jump(smart_enemy, SES_ATTACK, SMART_ENEMY_ATTACK_WAIT_TIME);

            return;
        }
    } else {
        extra->track.seconds_seeing = 0;
    }

    switch (extra->state) {
        case SES_WATCH:
        case SES_CONFIRM:
            // Count down seconds watching
            extra->seconds -= delta_time;

            // If we're done watching
            if (extra->seconds <= 0) {
                // If we are confirming, return to our watchpoint
                if (extra->state == SES_CONFIRM) {
                    smart_enemy_transition_untimed(smart_enemy, SES_MIGRATE);
                    smart_enemy_goto_watchpoint(smart_enemy);
                    return;
                }

                // We are at the end of our watch, choose another watchpoint to go to
                int watchpoint_to = smart_enemy_choose_watchpoint();

                // If no point is available, wait again
                if (watchpoint_to == -1) {
                    extra->seconds += SMART_ENEMY_WATCH_TIME;
                    return;
                }

                // Migrate to next watchpoint
                smart_enemy_transition_untimed(smart_enemy, SES_MIGRATE);
                smart_enemy_vacate_watchpoint(smart_enemy);
                smart_enemy_occupy_watchpoint(smart_enemy, watchpoint_to);
                smart_enemy_goto_watchpoint(smart_enemy);
            }

            break;
        case SES_MIGRATE:
        case SES_CHECK:
            {
                // If we have a route, go to the next point
                float dx = extra->route.x_vel * delta_time;
                float dy = extra->route.y_vel * delta_time;

                float *point = extra->route.points + ((extra->route.num_points - 1) * 2);

                float x_rem = fabsf(smart_enemy->x - point[0]);
                float y_rem = fabsf(smart_enemy->y - point[1]);

                // If we have passed the point, place us at it and prepare to go to next
                if (
                    (fabsf(dx) + x_rem > 0.05f && fabsf(dx) >= x_rem) || 
                    (fabsf(dy) + y_rem > 0.05f && fabsf(dy) >= y_rem)
                ) {
                    smart_enemy->x = point[0];
                    smart_enemy->y = point[1];
                    smart_enemy_pop_route_points(extra, 1);

                    // If there are more points, point towards the next one
                    if (extra->route.num_points) {
                        smart_enemy_path_smooth(smart_enemy);
                        
                        // Face next point
                        point = extra->route.points + ((extra->route.num_points - 1) * 2);
                        smart_enemy_pathfind_face_point(smart_enemy, point);
                    } else {
                        // We have finished our path

                        // If we are moving to a watchpoint, set our
                        // angle to be that of the point
                        if (extra->state == SES_MIGRATE) {
                            smart_enemy->angle = se_watchpoints[extra->route.watchpoint_index].angle;

                            // Transition to watch state
                            smart_enemy_transition_timed(smart_enemy, SES_WATCH, SMART_ENEMY_WATCH_TIME);
                        } else {
                            // Otherwise we are checking for the player, so confirm
                            // they are gone
                            smart_enemy_transition_timed(smart_enemy, SES_CONFIRM, SMART_ENEMY_CHECK_WAIT_TIME);
                        }
                    }
                } else {
                    smart_enemy->x += dx;
                    smart_enemy->y += dy;
                }
            }
            break;
        case SES_ATTACK:
            extra->seconds -= delta_time;

            char can_see_player = mobj_sees_player(smart_enemy);

            // If the attack delay is over
            if (extra->seconds <= 0) {
                // If we can see the player, attack
                if (can_see_player) {
                    shot_create(
                        smart_enemy->x,
                        smart_enemy->y,
                        smart_enemy->z + SMART_ENEMY_SHOT_HEIGHT_OFFSET,
                        smart_enemy->angle,
                        smart_enemy
                    );
                    sound_play_pos_mobj(sound_laser, 0.5f, smart_enemy);

                    // Prepare to attack again
                    extra->seconds += SMART_ENEMY_ATTACK_WAIT_TIME;
                } else {
                    // If we can't see the player, chase to where we last saw them
                    smart_enemy_transition_untimed(smart_enemy, SES_CHECK);
                    smart_enemy_goto_coords(
                        smart_enemy,
                        extra->track.last_player_x,
                        extra->track.last_player_y
                    );
                }
            } else if (can_see_player) {
                // If we can see the player, remember where we saw them
                smart_enemy_track_player(smart_enemy);
            }

            break;
        case SES_FLINCH:
            extra->seconds -= delta_time;

            // If we are done flinching, return to attacking
            if (extra->seconds <= 0) {
                smart_enemy_transition_timed(smart_enemy, SES_ATTACK, SMART_ENEMY_ATTACK_WAIT_TIME);
                smart_enemy->sprite_index = SMART_ENEMY_SPRITE_DEFAULT;
            } else if (mobj_sees_player(smart_enemy)) {
                // Remember where we last saw the player
                smart_enemy_track_player(smart_enemy);
            }

            break;
    }
}

void smart_enemy_destroy(mobj *smart_enemy) {
    __def_extra(smart_enemy);

    // If we are moving, free up the points
    if (extra->state == SES_MIGRATE || extra->state == SES_CHECK) {
        free(extra->route.points);
    }

    // If we are occupying a wachpoint, vacate
    if (extra->route.watchpoint_index != -1) {
        smart_enemy_vacate_watchpoint(smart_enemy);
    }

    mobj_destroy(smart_enemy);
}

void smart_enemy_damage(mobj *smart_enemy, int damage) {
    __def_extra(smart_enemy);

    extra->health -= damage;

    if (extra->health <= 0) {
        smart_enemy_destroy(smart_enemy);
        defeated_enemies++;
    } else {
        // If we are still alive, flinch
        extra->state = SES_FLINCH;
        extra->seconds = SMART_ENEMY_FLINCH_TIME;
        smart_enemy->sprite_index = SMART_ENEMY_SPRITE_FLINCH;
        sound_play_pos_mobj(sound_enemyFlinch, 1.0f, smart_enemy);
    }
}

void (*mobj_behaviors[NUM_MOBJ_TYPES])(mobj *) = {
    NULL, // notype
    particle_behave,
    enemy_behave,
    smart_enemy_behave
};

unsigned long mobj_extra_sizes[NUM_MOBJ_TYPES] = {
    0, // notype
    sizeof(particle_extra),
    sizeof(enemy_extra),
    sizeof(smart_enemy_extra)
};