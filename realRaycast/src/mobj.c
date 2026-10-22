#include <mobj.h>
#include <behavior.h>

__linked_list_head__(mobj)

mobj *get_mobj_head(void) {
    return mobj_head;
}

__doubly_linked_list_destroyer__(
    mobj,
    if (item->extra) {
        free(item->extra);
    }
    if (item->animation) {
        free(item->animation);
    }
)

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
        if (get_mobj_extra_size(type)) {
            item->extra = malloc(get_mobj_extra_size(type));
        } else {
            item->extra = NULL;
        }
)

__linked_list_destroy_all__(mobj);

void mobj_behave(mobj *o) {
    mobj_behavior b = get_mobj_behavior(o->type);
    if (b) {
        b(o);
    }
}