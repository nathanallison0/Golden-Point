#include "../../SDL/SDL3Start.h"
#include "../linkedList.h"

#define SHOT_SPRITE_COUNT 20
#define KEY_SHOT SDL_SCANCODE_SPACE
#define SHOT_SPEED (GRID_SPACING * 300)
#define SHOT_LENGTH (GRID_SPACING * 2)

__doubly_linked_list_all_add__(
    shot,
        float x1; float y1;
        float x2; float y2;
        float z; float angle;
        float x_mom; float y_mom;
        float x_spr_incr; float y_spr_incr,
    (float x1, float y1, float z, float length, float angle),
        item->x1 = x1;
        item->y1 = y1;
        item->z = z;
        item->x2 = x1 + (cosf(angle) * length);
        item->y2 = y1 + (sinf(angle) * length);
        item->angle = angle;
        item->x_spr_incr = (item->x2 - x1) / (SHOT_SPRITE_COUNT - 1);
        item->y_spr_incr = (item->y2 - y1) / (SHOT_SPRITE_COUNT - 1);
        item->x_mom = cosf(angle) * SHOT_SPEED;
        item->y_mom = sinf(angle) * SHOT_SPEED;
    ,;
)

#define try_mobj_hit(x, y) \
float dist = point_dist(s->x1, s->y1, x, y); \
if (dist < hit_dist) { hit_mobj = o; hit_x = x; hit_y = y; hit_dist = dist; }

void shot_advance(shot *s) {
    float x_dist = s->x_mom * delta_time;
    float abs_x_dist = fabsf(x_dist);
    float x_to = s->x1 + x_dist;
    
    // Determine with a raycast if we will hit a wall
    raycast_info v;
    ray_hit result = raycast_to_x(s->x1, s->y1, x_to, s->angle, &v);

    // If we will, only travel as far as the wall
    if (result == RAY_HORIZHIT) {
        x_to = v.c_hx;
    } else if (result == RAY_VERTHIT) {
        x_to = v.c_vx * GRID_SPACING;
    }

    // Loop through all mobjs
    //mobj_hit *mobj_hits = NULL;
    float hit_dist = __FLT_MAX__;
    float hit_x, hit_y;
    mobj *hit_mobj = NULL;
    for (mobj *o = mobj_head; o; o = o->next) {
        // Check if mobj is within x range
        float tan_angle = tanf(s->angle);

        if (v.quadrant == 1 || v.quadrant == 4) {
            if (
                (abs_x_dist > o->radius && (range(s->x1, <=, o->x - o->radius, <=, x_to) || range(s->x1, <=, o->x + o->radius, <=, x_to))) ||
                (abs_x_dist <= o->radius && (range(o->x - o->radius, <=, s->x1, <=, o->x + o->radius) || range(o->x - o->radius, <=, x_to, <=, o->x + o->radius)))
            ) {
                // Check relevant x and y side based off quadrant for a match

                // Check x side
                float left_y_hit = s->y1 + (tan_angle * (o->x - o->radius - s->x1));
                if (
                    (v.quadrant == 1 && left_y_hit > o->y + o->radius) || 
                    (v.quadrant == 4 && left_y_hit < o->y - o->radius)
                ) {
                    // Miss
                    continue;
                }

                if (range(o->y - o->radius, <=, left_y_hit, <=, o->y + o->radius)) {
                    //add_mobj_hit(o->x - o->radius, left_y_hit, o);
                    try_mobj_hit(o->x - o->radius, left_y_hit);
                    continue;
                }

                // Check y side
                float right_y_hit = s->y1 + (tan_angle * (o->x + o->radius - s->x1));
                if (v.quadrant == 1) {
                    if (left_y_hit < o->y - o->radius && right_y_hit >= o->y - o->radius) {
                        float x_hit = s->x1 + ((o->y - o->radius - s->y1) / tan_angle);
                        if (x_hit <= x_to) {
                            //add_mobj_hit(x_hit, o->y - o->radius, o);
                            try_mobj_hit(x_hit, o->y - o->radius);
                        }
                    }
                } else {
                    if (left_y_hit > o->y + o->radius && right_y_hit <= o->y + o->radius) {
                        float x_hit = s->x1 - ((s->y1 - o->y - o->radius) / tan_angle);
                        if (x_hit <= x_to) {
                            //add_mobj_hit(x_hit, o->y + o->radius, o);
                            try_mobj_hit(x_hit, o->y + o->radius);
                        }
                    }
                }
            }
        } else {
            if (
                (abs_x_dist > o->radius && (range(x_to, <=, o->x - o->radius, <=, s->x1) || range(x_to, <=, o->x + o->radius, <=, s->x1))) ||
                (abs_x_dist <= o->radius && (range(o->x - o->radius, <=, s->x1, <=, o->x + o->radius) || range(o->x - o->radius, <=, x_to, <=, o->x + o->radius)))
            ) {
                // Check relevant x and y side based off quadrant for a match

                // Check x side
                float right_y_hit = s->y1 - (tan_angle * (s->x1 - o->x - o->radius));
                if (
                    (v.quadrant == 2 && right_y_hit > o->y + o->radius) || 
                    (v.quadrant == 3 && right_y_hit < o->y - o->radius)
                ) {
                    // Miss
                    continue;
                }

                if (range(o->y - o->radius, <=, right_y_hit, <=, o->y + o->radius)) {
                    //add_mobj_hit(o->x + o->radius, right_y_hit, o);
                    try_mobj_hit(o->x + o->radius, right_y_hit);
                    continue;
                }

                // Check y side
                float left_y_hit = s->y1 - (tan_angle * (s->x1 - o->x + o->radius));
                if (v.quadrant == 2) {
                    if (right_y_hit < o->y - o->radius && left_y_hit >= o->y - o->radius) {
                        float x_hit = s->x1 + ((o->y - o->radius - s->y1) / tan_angle);
                        if (x_hit >= x_to) {
                            //add_mobj_hit(x_hit, o->y - o->radius, o);
                            try_mobj_hit(x_hit, o->y - o->radius);
                        }
                    }
                } else {
                    if (right_y_hit > o->y + o->radius && left_y_hit <= o->y + o->radius) {
                        float x_hit = s->x1 - ((s->y1 - o->y - o->radius) / tan_angle);
                        if (x_hit >= x_to) {
                            //add_mobj_hit(x_hit, o->y + o->radius, o);
                            try_mobj_hit(x_hit, o->y + o->radius);
                        }
                    }
                }
            }
        }
    }

    // If we hit a mobj, make effect
    if (hit_mobj) {
        mobj_create(MOBJ_NOTYPE, hit_x, hit_y, s->z, 0, 20, sprite_guyForGame, 0);
        shot_destroy(s);
        return;
    }

    // If no walls were hit, move shot
    if (result == RAY_NOHIT) {
        float y_dist = s->y_mom * delta_time;
        s->x1 = x_to;
        s->x2 += x_dist;
        s->y1 += y_dist;
        s->y2 += y_dist;
        return;
    }

    // Otherwise, make effect at wall
    if (result == RAY_HORIZHIT) {
        char is_up = v.quadrant == 1 || v.quadrant == 2;
        particle_init(v.c_hx, (v.c_hy * GRID_SPACING), s->z, sprite_smoke, is_up ? 1 : 3);
    } else {
        char is_right = v.quadrant == 1 || v.quadrant == 4;
        particle_init((v.c_vx * GRID_SPACING), v.c_vy, s->z, sprite_smoke, is_right ? 0 : 2);
    }
    shot_destroy(s);
}