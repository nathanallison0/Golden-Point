#ifndef _GPOINT_RAYCASTS_H_
#define _GPOINT_RAYCASTS_H_

#include <SDL3Start.h>
#include <map.h>
#include <mobj.h>

#define RAY_SUCCESS 0

typedef struct {
    float x;
    float y;
} xy;

typedef struct {
    door *door;
    float x;
    float y;
} door_hit_info;

typedef struct {
    char quadrant;
    float c_hx; int c_hy;
    float d_hx; char d_hy;
    int c_vx; float c_vy;
    char d_vx; float d_vy;
    float c_epsilon_h, d_epsilon_h;
    float c_epsilon_v, d_epsilon_v;
    door_hit_info door_hit;
    float texture_offset;
} raycast_info;

enum {
    RAY_NOHIT,
    RAY_HORIZHIT,
    RAY_VERTHIT
};
typedef Uint8 ray_hit;

xy raycast(float x, float y, float angle, raycast_info *v, int *texture_index, int *texture_col);

ray_hit raycast_to_angle(float x1, float y1, float x2, float y2, float angle, raycast_info *v);

ray_hit raycast_to(float x1, float y1, float x2, float y2, raycast_info *v);

ray_hit raycast_to_x(float x1, float y1, float x2, float angle, raycast_info *v);

ray_hit raycast_to_mobj(mobj *start, mobj *end, raycast_info *v);

#endif