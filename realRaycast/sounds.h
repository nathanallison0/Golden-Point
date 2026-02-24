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
#include <SDL3_mixer/SDL_mixer.h>
#include <stdio.h>
#include <dirent.h>

static MIX_Mixer *audio_mixer;
static MIX_Track **tracks_to_destroy = NULL;
static int num_tracks_to_destroy = 0;

MIX_Audio **sounds;
int num_sounds = 0;
#define SOUND_DIR "./sounds"

enum {
    sound_laser,
    sound_door
};

void import_sounds(void) {
    DIR *sound_dir = opendir(SOUND_DIR);

    if (sound_dir) {
        struct dirent *entry;
        while ((entry = readdir(sound_dir))) {
            // Don't handle . and ..
            if (entry->d_type != DT_DIR) {
                printf("loading sound %s\n", entry->d_name);
                num_sounds++;
                sounds = realloc(sounds, num_sounds * sizeof(MIX_Audio *));

                // Construct rel path to file
                char *path;
                asprintf(&path, SOUND_DIR "/%s", entry->d_name);
                printf("rel path: %s\n", path);

                // Load sound
                sounds[num_sounds - 1] = MIX_LoadAudio(audio_mixer, path, false);
                free(path);
            }
        }
        closedir(sound_dir);
    } else {
        fprintf(stderr, "Could not open sound dir: '" SOUND_DIR "'\n");
    }
}

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
    import_sounds();
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

void sound_play_static(int sound_index, float gain) {
    create_singleuse_track(sounds[sound_index], gain, NULL);
}

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

void sound_play_pos_static(int sound_index, float gain, float x, float y, float z) {
    pos_sound *p_sound = pos_sound_create(sounds[sound_index], gain);

    p_sound->type = POS_SOUND_STATIC;
    p_sound->pos.coords.x = x;
    p_sound->pos.coords.y = y;
    p_sound->pos.coords.z = z;
    sound_update_pos_pan(p_sound);
}

void sound_play_pos_mobj(int sound_index, float gain, mobj *obj) {
    pos_sound *p_sound = pos_sound_create(sounds[sound_index], gain);
    
    p_sound->type = POS_SOUND_MOBJ;
    p_sound->pos.obj = obj;
    sound_update_pos_pan(p_sound);
}
#endif