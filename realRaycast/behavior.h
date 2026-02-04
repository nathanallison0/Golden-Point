shot *shot_create(float, float, float, float, mobj *);

char mobj_turn_towards(mobj *o, float target_angle, float speed) {
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
#define ENEMY_HEALTH 100
void enemy_init(float x, float y, float z, float angle) {
    mobj *enemy = mobj_create(MOBJ_ENEMY, x, y, z, angle, ENEMY_RADIUS, sprite_boxShaded, MF_SHOOTABLE);
    __def_extra(enemy);
    extra->health = ENEMY_HEALTH;
    extra->neutral_angle = angle;
}

#define ENEMY_RANGE (GRID_SPACING * 10)
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
        point_dist(enemy->x, enemy->y, player->x, player->y) <= ENEMY_RANGE &&
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
        sound_play_pos_mobj(sound_effect, 1.0f, enemy);
    }
}

void (*mobj_behaviors[NUM_MOBJ_TYPES])(mobj *) = {
    NULL, // notype
    particle_behave, // particle
    enemy_behave // enemy
};

Uint8 mobj_extra_sizes[NUM_MOBJ_TYPES] = {
    0, // notype
    sizeof(particle_extra), // particle
    sizeof(enemy_extra) // enemy
};