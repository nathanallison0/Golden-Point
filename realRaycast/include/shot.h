#ifndef _GPOINT_SHOT_H_
#define _GPOINT_SHOT_H_

#include <SDL3Start.h>
#include <linkedList.h>
#include <constants.h>
#include <stdlib.h>

typedef struct mobj mobj;
typedef Uint8 mobj_type;

#define SHOT_SPRITE_COUNT 20
#define SHOT_SPEED (GRID_SPACING * 200)
#define SHOT_LENGTH (GRID_SPACING * 2)
#define SHOT_START_DISP 2

__doubly_linked_list_init__(
    shot,
        float x1; float y1;
        float x2; float y2;
        float z; float angle;
        float x_mom; float y_mom;
        mobj *src;
        float x_spr_incr; float y_spr_incr
)

shot *get_shot_head(void);

shot *shot_create(float x1, float y1, float z, float angle, mobj *src);

void shot_destroy(shot *item);

void shot_destroy_all(void);

void shot_advance(shot *s);

#endif