#ifndef _GPOINT_SOUNDS_H_
#define _GPOINT_SOUNDS_H_

#if __EMSCRIPTEN__
#define import_sounds()
#define init_sound()
#define print_pos_sound(s)
#define sound_clear_finished()
#define sound_play_static(a, b)
#define sound_update_pos_pan(a)
#define sound_play_pos_static(a, b, c, d, e)
#define sound_play_pos_mobj(a, b, c)
#else
#include <mobj.h>
#include <linkedList.h>
#include <constants.h>
#include <SDL3_mixer/SDL_mixer.h>

enum {
    sound_laser,
    sound_door,
    sound_enemyFlinch
};

#define POS_SOUND_STATIC 0
#define POS_SOUND_MOBJ 1

__doubly_linked_list_init__(
    pos_sound,
        MIX_Track *track;
        Uint8 type;
        union {
            mobj *obj;

            struct {
                float x;
                float y;
                float z;
            } coords;
        } pos;
)

void init_sound(void);

void print_pos_sound(pos_sound *s);

void update_sounds_pos_pan(void);

void sound_clear_finished(void);

void sound_play_static(int sound_index, float gain);

void sound_update_pos_pan(pos_sound *s);

void sound_play_pos_static(int sound_index, float gain, float x, float y, float z);

void sound_play_pos_mobj(int sound_index, float gain, mobj *obj);

#endif
#endif