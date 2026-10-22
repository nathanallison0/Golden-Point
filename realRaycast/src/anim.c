#include <anim.h>
#include <mobj.h>
#include <sprites.h>
#include <spritesRaw.h>
#include <animDescsRaw.h>

__linked_list_head__(anim)

__linked_list_creator_add__(
    anim,
    (anim_type type, Uint16 anim_desc_index),
        item->type = type;
        item->frames = 0;
        item->sub_frames = 0;
        item->anim_desc_index = anim_desc_index
)

__linked_list_destroy_all__(anim)

void anim_start(mobj *o, anim_type type, Uint16 desc_index) {
    // Destroy any existing animation
    if (o->animation) {
        free(o->animation);
    }
    o->animation = anim_create(type, desc_index);
    o->sprite_index = anim_descs[desc_index].start_frame;
}

void anim_advance(mobj *o) {
    anim *a = o->animation;
    anim_desc *desc = anim_descs + a->anim_desc_index;
    a->sub_frames++;
    if (a->sub_frames == desc->sub_frames) {
        a->sub_frames = 0;
        a->frames++;
        if (a->frames == desc->frames) {
            switch (a->type) {
                case ANIM_ONCE:
                    free(a);
                    o->animation = NULL;
                    break;
                case ANIM_REPEAT:
                    a->frames = 0;
                    o->sprite_index = desc->start_frame;
                    break;
                case ANIM_ONCE_DESTROY:
                    mobj_destroy(o);
                    break;
            }
        } else {
            if (sprites[o->sprite_index].is_rot) {
                o->sprite_index += NUM_ROT_SPRITE_FRAMES;
            } else {
                o->sprite_index++;
            }
        }
    }
}