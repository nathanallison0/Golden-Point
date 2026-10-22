#ifndef _GPOINT_MOBJ_H_
#define _GPOINT_MOBJ_H_

#include <anim.h>
#include <linkedList.h>
#include <behavior.h>
#include <extra.h>
#include <stdlib.h>

typedef struct anim anim;

enum {
    MOBJ_NOTYPE,
    MOBJ_PARTICLE,
    MOBJ_ENEMY,
    MOBJ_SMART_ENEMY,
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

mobj *get_mobj_head(void);

void mobj_destroy(mobj *);

mobj *mobj_create(mobj_type type, float x, float y, float z, float angle, Uint8 radius, Uint16 sprite_index, Uint8 flags);

void mobj_destroy_all(void);

void mobj_behave(mobj *o);

#endif