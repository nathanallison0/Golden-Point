#ifndef _GPOINT_ANIM_H_
#define _GPOINT_ANIM_H_

#include <SDL3Start.h>
#include <linkedList.h>
#include <mobj.h>
#include <stdlib.h>

typedef struct mobj mobj;
typedef Uint8 mobj_type;

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

__linked_list_init__(
    anim,
        anim_type type;
        Uint16 frames;
        Uint16 sub_frames;
        Uint16 anim_desc_index
)

anim *get_anim_head(void);

anim *anim_create(anim_type type, Uint16 anim_desc_index);

void anim_destroy_all(void);

/**
 * Start an animation for a mobj.
 * 
 * \param o The object to animate
 * \param type The type of animation
 * \param desc_index The index of the animation description to use
 */
void anim_start(mobj *o, anim_type type, Uint16 desc_index);

/**
 * Advance the animation of an object by one sub-frame.
 * \param o The object to animate
 */
void anim_advance(mobj *o);

#endif