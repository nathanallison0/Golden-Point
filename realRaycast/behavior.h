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

    /* fill_dgp(o->x + x1_off, o->y + y1_off, DG_RED);
    fill_dgp(o->x + x2_off, o->y + y2_off, DG_RED);

    fill_dgp(x + x1_off, y + y1_off, DG_GREEN);
    fill_dgp(x + x2_off, y + y2_off, DG_GREEN); */

    raycast_info v;
    return (
        raycast_to(o->x + x1_off, o->y + y1_off, x + x1_off, y + y1_off, angle, &v) == RAY_NOHIT &&
        raycast_to(o->x + x2_off, o->y + y2_off, x + x2_off, y + y2_off, angle, &v) == RAY_NOHIT
    );
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
#define SMART_ENEMY_RADIUS 20
#define SMART_ENEMY_SPEED 200
enum {
    SES_WATCH,
    SES_MIGRATE
};
typedef Uint8 se_state;

extra (
    smart_enemy,
    float x_vel; float y_vel;
    struct {
        float *points;
        int num_points;
    } route;
);

void smart_enemy_face_point(mobj *smart_enemy, float x, float y) {
    __def_extra(smart_enemy);
    smart_enemy->angle = get_angle_to(smart_enemy->x, smart_enemy->y, x, y);
    extra->x_vel = cosf(smart_enemy->angle) * SMART_ENEMY_SPEED;
    extra->y_vel = sinf(smart_enemy->angle) * SMART_ENEMY_SPEED;
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

void smart_enemy_goto(mobj *smart_enemy, float x, float y) {
    __def_extra(smart_enemy);

    if (extra->route.num_points != 0) {
        free(extra->route.points);
    }

    extra->route.points = pathfind(
        (int) smart_enemy->x / GRID_SPACING, (int) smart_enemy->y / GRID_SPACING,
        (int) x / GRID_SPACING, (int) y / GRID_SPACING,
        &(extra->route.num_points)
    );

    smart_enemy_path_smooth(smart_enemy);

    float *point = extra->route.points + ((extra->route.num_points - 1) * 2);

    if (extra->route.points) {
        smart_enemy_face_point(
            smart_enemy,
            point[0],
            point[1]
        );
    }

    // Show pathing
    for (int i = 0; i < extra->route.num_points; i++) {
        temp_dgp(
            extra->route.points[i * 2],
            extra->route.points[(i * 2) + 1],
            DG_RED
        );
    }
}

void smart_enemy_init(float x, float y) {
    mobj *smart_enemy = mobj_create(
        MOBJ_SMART_ENEMY, x, y, 0, 0, SMART_ENEMY_RADIUS, sprite_boxShaded, MF_SHOOTABLE
    );

    __def_extra(smart_enemy);
    extra->route.num_points = 0;
}

void smart_enemy_behave(mobj *smart_enemy) {
    __def_extra(smart_enemy);

    // If we have a route, go to the next point
    if (extra->route.num_points != 0) {
        float dx = extra->x_vel * delta_time;
        float dy = extra->y_vel * delta_time;

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
                smart_enemy_face_point(smart_enemy, point[0], point[1]);
            }
        } else {
            smart_enemy->x += dx;
            smart_enemy->y += dy;
        }
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