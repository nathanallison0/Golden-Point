#ifndef _GPOINT_MAP_H_
#define _GPOINT_MAP_H_

#include <SDL3Start.h>

#define GRID_WIDTH 25
#define GRID_HEIGHT 25

enum {
    TILE_NONSOLID,
    TILE_SOLID,
    TILE_HORIZ_DOOR
};
typedef Uint8 tile;

#define NUM_DOORS 1

#define DOORF_MOVING 1
#define DOORF_OPENING (1 << 1)

typedef struct {
    int x, y;
    float progress;
    Uint8 flags;
} door;

Uint8 get_map(int x, int y);

Uint8 get_map_coords(float x, float y);

#define coord_to_grid(x) ((int) x / GRID_SPACING)

Uint8 get_horiz_texture(int x, int y);

Uint8 get_vert_texture(int x, int y);

Uint8 get_floor_texture(int x, int y);

Uint8 get_ceil_texture(int x, int y);

door *get_door(int index);

door *get_door_tile(int x, int y);

door *get_door_coords(float x, float y);

#endif