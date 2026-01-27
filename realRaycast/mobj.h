#include "../linkedList.h"
#include <SDL3/SDL.h>

enum {
    MOBJ_NOTYPE,
    MOBJ_TESTER,
    MOBJ_PARTICLE,
    NUM_MOBJ_TYPES
};
typedef Uint8 mobj_type;

__doubly_linked_list_init__(
    mobj,
        mobj_type type;
        Uint8 flags;
        Uint16 sprite_index;
        float x;
        float y;
        float z;
        float angle;
        Uint8 radius;
        void *extra
)

Uint8 mobj_extra_sizes[NUM_MOBJ_TYPES];

__doubly_linked_list_creator_add__(
    mobj,
    (mobj_type type, float x, float y, float z, float angle, Uint8 radius, Uint16 sprite_index, Uint8 flags),
        item->type = type;
        item->x = x;
        item->y = y;
        item->z = z;
        item->angle = angle;
        item->radius = radius;
        item->sprite_index = sprite_index;
        item->flags = flags;

        // Allocate space for extra data
        if (mobj_extra_sizes[type]) {
            item->extra = malloc(mobj_extra_sizes[type]);
        } else {
            item->extra = NULL;
        }
)

__doubly_linked_list_destroyer__(
    mobj,
    if (item->extra) {
        free(item->extra);
    }
)

__linked_list_destroy_all__(mobj)

/* __doubly_linked_list_all_add__(
    mobj,
        mobj_type type;
        Uint8 flags;
        Uint16 sprite_index;
        float x;
        float y;
        float z;
        float angle;
        Uint8 radius;
        void *extra,
    (mobj_type type, float x, float y, float z, float angle, Uint8 radius, Uint16 sprite_index, Uint8 flags, ...),
        item->type = type;
        item->x = x;
        item->y = y;
        item->z = z;
        item->angle = angle;
        item->radius = radius;
        item->sprite_index = sprite_index;
        item->flags = flags;

        // Allocate space for extra data
        if (mobj_extra_sizes[type]) {
            item->extra = malloc(mobj_extra_sizes[type]);
        } else {
            item->extra = NULL;
        }

        // Call initializer
        va_list args;
        va_start(args, flags);
        if (mobj_initializers[type]) {
            mobj_initializers[type](item, args);
        }
        va_end(args),

    // On destroy
    if (item->extra) {
        free(item->extra);
    }
) */

#define __def_extra(type) type##_extra *extra = (type##_extra *) type->extra;

// Tester
#define TESTER_LOOK_SPEED (PI / 2)
void tester_behave(mobj *tester) {
    // Look towards player
    float angle_to_player = get_angle_to_player(tester->x, tester->y);
    float angle_diff = angle_to_player - tester->angle;
    fix_angle_180(angle_diff);

    float delta = TESTER_LOOK_SPEED * delta_time;
    
    if (fabsf(angle_diff) <= delta) {
        tester->angle = angle_to_player;
    } else {
        tester->angle += (angle_diff < 0 ? -delta : delta);
    }

    fix_angle_180(tester->angle);
}

// Particle
typedef struct {
    Uint8 frames;
    float x_vel;
    float y_vel;
    float z_vel;
} particle_extra;

Uint8 mobj_extra_sizes[NUM_MOBJ_TYPES] = {
    0, // notype
    0, // tester
    sizeof(particle_extra)
};

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

void (*mobj_behaviors[NUM_MOBJ_TYPES])(mobj *) = {
    NULL, // notype
    tester_behave, // tester
    particle_behave // particle
};

#define MOBJ_FLAG_SOLID 1
#define MOBJ_FLAG_SHOOTABLE (1 << 1)