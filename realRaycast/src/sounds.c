#include <sounds.h>
#include <stdio.h>
#include <dirent.h>
#include <engine.h>

MIX_Mixer *audio_mixer;
MIX_Track **tracks_to_destroy = NULL;
int num_tracks_to_destroy = 0;

MIX_Audio **sounds;
int num_sounds = 0;
#define SOUND_DIR "./sounds"

void import_sounds(void) {
    DIR *sound_dir = opendir(SOUND_DIR);

    if (sound_dir) {
        struct dirent *entry;
        while ((entry = readdir(sound_dir))) {
            // Don't handle . and ..
            if (entry->d_type != DT_DIR) {
                num_sounds++;
                sounds = realloc(sounds, num_sounds * sizeof(MIX_Audio *));

                // Construct rel path to file
                char *path;
                asprintf(&path, SOUND_DIR "/%s", entry->d_name);

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

__linked_list_head__(pos_sound)

__doubly_linked_list_destroyer__(
    pos_sound,
    /* default destroyer */;
)

void destroy_track_callback(void *data, MIX_Track *track) {
    num_tracks_to_destroy++;
    tracks_to_destroy = realloc(tracks_to_destroy, num_tracks_to_destroy * sizeof(MIX_Track *));
    tracks_to_destroy[num_tracks_to_destroy - 1] = track;

    // If the sound is a positional sound, destroy that structure as well
    if (data) {
        pos_sound_destroy((pos_sound *) data);
    }
}

MIX_Track *create_singleuse_track(MIX_Audio *audio, float gain, pos_sound *pos_sound_data) {
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
        .x = -(dist_to * cosf(rel_angle_to)) / (GRID_SPACING * 2),
        .z = (dist_to * sinf(rel_angle_to)) / (GRID_SPACING * 2),
        .y = 0
    });
}

void update_sounds_pos_pan(void) {
    for (pos_sound *s = pos_sound_head; s; s = s->next) {
        sound_update_pos_pan(s);
    }
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