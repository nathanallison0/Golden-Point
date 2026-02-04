#include "anim.h"

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
        item->animation = NULL;

        // Allocate space for extra data
        if (mobj_extra_sizes[type]) {
            item->extra = malloc(mobj_extra_sizes[type]);
        } else {
            item->extra = NULL;
        }
)

__linked_list_destroy_all__(mobj)

void create_assign_player(void) {
    player = mobj_create(MOBJ_NOTYPE, PLAYER_START_X, PLAYER_START_Y, PLAYER_START_Z, PLAYER_START_ANGLE, PLAYER_RADIUS, 0, MF_SHOOTABLE);
}

#define extra(type, attrs) typedef struct {attrs} type##_extra
#define __def_extra_var(type, var) type##_extra *extra = (type##_extra *) var->extra
#define __def_extra(type) __def_extra_var(type, type)

void particle_init(float, float, float, Uint16, Uint8);