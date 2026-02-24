#if __EMSCRIPTEN__
#define init_sound()
#define print_pos_sound(s)
#define sound_clear_finished()
#define sound_play_static(a, b)
#define sound_update_pos_pan(a)
#define sound_play_pos_static(a, b, c, d, e)
#define sound_play_pos_mobj(a, b, c)
#else
#include <SDL3_mixer/SDL_mixer.h>
#include <stdio.h>

static MIX_Mixer *audio_mixer;
static MIX_Track **tracks_to_destroy = NULL;
static Uint8 num_tracks_to_destroy = 0;

MIX_Audio *sound_effect;

#define POS_SOUND_STATIC 0
#define POS_SOUND_MOBJ 1

#define MAX_SOUND_RADIUS (GRID_SPACING * 20)

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
        } pos
)

__doubly_linked_list_head__(pos_sound)

__doubly_linked_list_destroyer__(
    pos_sound,
    ; // no extra destroyer code
)

static void destroy_track_callback(void *data, MIX_Track *track) {
    num_tracks_to_destroy++;
    tracks_to_destroy = realloc(tracks_to_destroy, num_tracks_to_destroy * sizeof(MIX_Track *));
    tracks_to_destroy[num_tracks_to_destroy - 1] = track;

    // If the sound is a positional sound, destroy that structure as well
    if (data) {
        pos_sound_destroy((pos_sound *) data);
    }
}

static MIX_Track *create_singleuse_track(MIX_Audio *audio, float gain, pos_sound *pos_sound_data) {
    MIX_Track *track = MIX_CreateTrack(audio_mixer);

    // If this is a positional sound, give reference to structure in callback
    MIX_SetTrackStoppedCallback(track, *destroy_track_callback, pos_sound_data);
    MIX_SetTrackGain(track, gain);
    MIX_SetTrackAudio(track, audio);
    MIX_PlayTrack(track, 0);

    return track;
}

__doubly_linked_list_creator_add__(
    pos_sound,
    (MIX_Audio *audio, float gain),
        item->track = create_singleuse_track(audio, gain, item);
)

void init_sound(void) {
    MIX_Init();
    audio_mixer = MIX_CreateMixerDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, NULL);
}

void print_pos_sound(pos_sound *s) {
    printf(
        "pos_sound track=%p type=%d next=%p prev=%p\n",
        s->track, s->type, s->next, s->prev
    );
}

void sound_clear_finished(void) {
    for (Uint8 i = 0; i < num_tracks_to_destroy; i++) {
        MIX_DestroyTrack(tracks_to_destroy[i]);
    }
    num_tracks_to_destroy = 0;
}

void sound_play_static(MIX_Audio *audio, float gain) {
    create_singleuse_track(audio, gain, NULL);
}

/* static pos_sound *create_pos_sound(MIX_Audio *audio, float gain) {
    pos_sound *p_sound = malloc(sizeof(pos_sound));
    if (pos_sound_head) {
        pos_sound_head->prev = p_sound;
    }
    p_sound->next = pos_sound_head;
    pos_sound_head = p_sound;
    p_sound->track = create_singleuse_track(audio, gain, p_sound);
    return p_sound;
} */

void sound_update_pos_pan(pos_sound *s) {
    float x, y;
    if (s->type == POS_SOUND_STATIC) {
        x = s->pos.coords.x;
        y = s->pos.coords.y;
    } else {
        x = s->pos.obj->x;
        y = s->pos.obj->y;
    }

    float rel_angle_to = get_angle_from_player(x, y) - (player->angle - PI_2);
    float dist_to = point_dist(player->x, player->y, x, y);

    MIX_SetTrack3DPosition(s->track, &(MIX_Point3D) {
        .x = -(dist_to * cosf(rel_angle_to)) / GRID_SPACING,
        .z = (dist_to * sinf(rel_angle_to)) / GRID_SPACING,
        .y = 0
    });
}

void sound_play_pos_static(MIX_Audio *audio, float gain, float x, float y, float z) {
    pos_sound *p_sound = pos_sound_create(audio, gain);

    p_sound->type = POS_SOUND_STATIC;
    p_sound->pos.coords.x = x;
    p_sound->pos.coords.y = y;
    p_sound->pos.coords.z = z;
    sound_update_pos_pan(p_sound);
}

void sound_play_pos_mobj(MIX_Audio *audio, float gain, mobj *obj) {
    pos_sound *p_sound = pos_sound_create(audio, gain);
    
    p_sound->type = POS_SOUND_MOBJ;
    p_sound->pos.obj = obj;
    sound_update_pos_pan(p_sound);
}
#endif