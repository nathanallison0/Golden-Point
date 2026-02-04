enum {
    ANIM_ONCE,
    ANIM_REPEAT,
    ANIM_ONCE_DESTROY
};
typedef Uint8 anim_type;

typedef struct {
    Uint16 start_frame;
    Uint16 frames;
    Uint16 sub_frames;
} anim_desc;

__linked_list_all_add__(
    anim,
        anim_type type;
        Uint16 frames;
        Uint16 sub_frames;
        Uint16 anim_desc_index,
    (anim_type type, Uint16 anim_desc_index),
        item->type = type;
        item->frames = 0;
        item->sub_frames = 0;
        item->anim_desc_index = anim_desc_index
)

enum {
    MOBJ_NOTYPE,
    MOBJ_PARTICLE,
    MOBJ_ENEMY,
    NUM_MOBJ_TYPES
};
typedef Uint8 mobj_type;

#define MF_SOLID 1
#define MF_SHOOTABLE (1 << 1)

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
        void *extra;
        anim *animation;
)

__doubly_linked_list_destroyer__(
    mobj,
    if (item->extra) {
        free(item->extra);
    }
    if (item->animation) {
        free(item->animation);
    }
)
mobj *mobj_create(mobj_type, float, float, float, float, Uint8, Uint16, Uint8);

#include "animDescsRaw.h"

#define SHOT_SPRITE_COUNT 20
#define SHOT_SPEED (GRID_SPACING * 200)
#define SHOT_LENGTH (GRID_SPACING * 2)
#define SHOT_START_DISP 2

__doubly_linked_list_all_add__(
    shot,
        float x1; float y1;
        float x2; float y2;
        float z; float angle;
        float x_mom; float y_mom;
        mobj *src;
        float x_spr_incr; float y_spr_incr,
    (float x1, float y1, float z, float angle, mobj *src),
        item->x1 = x1;
        item->y1 = y1;
        item->z = z;
        item->x2 = x1 + (cosf(angle) * SHOT_LENGTH);
        item->y2 = y1 + (sinf(angle) * SHOT_LENGTH);
        item->angle = angle;
        item->src = src;
        item->x_spr_incr = (item->x2 - x1) / (SHOT_SPRITE_COUNT - 1);
        item->y_spr_incr = (item->y2 - y1) / (SHOT_SPRITE_COUNT - 1);
        item->x_mom = cosf(angle) * SHOT_SPEED;
        item->y_mom = sinf(angle) * SHOT_SPEED,
    ;// No extra destroyer code
)