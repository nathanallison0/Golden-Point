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