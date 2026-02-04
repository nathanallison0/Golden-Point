#include "../../SDL/SDL3Start.h"
#include "../linkedList.h"

#define try_mobj_hit(x, y) \
float dist = point_dist(s->x1, s->y1, x, y); \
if (dist < hit_dist) { hit_mobj = o; hit_x = x; hit_y = y; hit_dist = dist; }

#define SHOT_DAMAGE 10
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
    float hit_dist = __FLT_MAX__;
    float hit_x, hit_y;
    mobj *hit_mobj = NULL;
    for (mobj *o = mobj_head; o; o = o->next) {
        // Only consider if shootable and not the source of the shot
        if (o == s->src || (o->flags & MF_SHOOTABLE) == 0) {
            continue;
        }

        // Check if mobj is within x range
        float tan_angle = tanf(s->angle);

        char within_radius_x = abs_x_dist <= o->radius * 2;
        if (v.quadrant == 1 || v.quadrant == 4) {
            if (
                (!within_radius_x && (range(s->x1, <=, o->x - o->radius, <=, x_to) || range(s->x1, <=, o->x + o->radius, <=, x_to))) ||
                (within_radius_x && (range(o->x - o->radius, <=, s->x1, <=, o->x + o->radius) || range(o->x - o->radius, <=, x_to, <=, o->x + o->radius)))
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

                if (range(o->y - o->radius, <=, left_y_hit, <=, o->y + o->radius) && o->x - o->radius >= s->x1) {
                    if (within_radius_x && (s->y1 < o->y - o->radius || s->y1 > o->y + o->radius)) {
                        continue;
                    }
                    try_mobj_hit(o->x - o->radius, left_y_hit);
                    continue;
                }

                // If the object's target top or bottom side is outside our y range, skip
                if (
                    (v.quadrant == 1 && o->y - o->radius <= s->y1) ||
                    (v.quadrant == 4 && o->y + o->radius >= s->y1)
                ) {
                    continue;
                }

                // Check y side
                float right_y_hit = s->y1 + (tan_angle * (o->x + o->radius - s->x1));
                if (v.quadrant == 1) {
                    if (left_y_hit < o->y - o->radius && right_y_hit >= o->y - o->radius) {
                        if (within_radius_x && o->y + o->radius < s->y1) {
                            continue;
                        }
                        float x_hit = s->x1 + ((o->y - o->radius - s->y1) / tan_angle);
                        if (x_hit <= x_to) {
                            try_mobj_hit(x_hit, o->y - o->radius);
                        }
                    }
                } else {
                    if (left_y_hit > o->y + o->radius && right_y_hit <= o->y + o->radius) {
                        if (within_radius_x && o->y - o->radius > s->y1) {
                            continue;
                        }
                        float x_hit = s->x1 - ((s->y1 - o->y - o->radius) / tan_angle);
                        if (x_hit <= x_to) {
                            try_mobj_hit(x_hit, o->y + o->radius);
                        }
                    }
                }
            }
        } else {
            if (
                (!within_radius_x && (range(x_to, <=, o->x - o->radius, <=, s->x1) || range(x_to, <=, o->x + o->radius, <=, s->x1))) ||
                (within_radius_x <= o->radius && (range(o->x - o->radius, <=, s->x1, <=, o->x + o->radius) || range(o->x - o->radius, <=, x_to, <=, o->x + o->radius)))
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

                if (range(o->y - o->radius, <=, right_y_hit, <=, o->y + o->radius) && o->x + o->radius <= s->x1) {
                    if (within_radius_x && (s->y1 < o->y - o->radius || s->y1 > o->y + o->radius)) {
                        continue;
                    }
                    try_mobj_hit(o->x + o->radius, right_y_hit);
                    continue;
                }

                // If the object's target top or bottom side is outside our y range, skip
                if (
                    (v.quadrant == 2 && o->y - o->radius <= s->y1) ||
                    (v.quadrant == 3 && o->y + o->radius >= s->y1)
                ) {
                    continue;
                }

                // Check y side
                float left_y_hit = s->y1 - (tan_angle * (s->x1 - o->x + o->radius));
                if (v.quadrant == 2) {
                    if (right_y_hit < o->y - o->radius && left_y_hit >= o->y - o->radius) {
                        if (within_radius_x && o->y + o->radius < s->y1) {
                            continue;
                        }
                        float x_hit = s->x1 + ((o->y - o->radius - s->y1) / tan_angle);
                        if (x_hit >= x_to) {
                            try_mobj_hit(x_hit, o->y - o->radius);
                        }
                    }
                } else {
                    if (right_y_hit > o->y + o->radius && left_y_hit <= o->y + o->radius) {
                        if (within_radius_x && o->y - o->radius > s->y1) {
                            continue;
                        }
                        float x_hit = s->x1 - ((s->y1 - o->y - o->radius) / tan_angle);
                        if (x_hit >= x_to) {
                            try_mobj_hit(x_hit, o->y + o->radius);
                        }
                    }
                }
            }
        }
    }

    // If we hit a mobj, make effect
    if (hit_mobj) {
        mobj *effect = mobj_create(MOBJ_NOTYPE, hit_x, hit_y, s->z, 0, 5, 0, 0);
        anim_start(effect, ANIM_ONCE_DESTROY, anim_laserHit);
        shot_destroy(s);
        

        // Deal damage to enemies and player
        if (hit_mobj->type == MOBJ_ENEMY) {
            __def_extra_var(enemy, hit_mobj);
            if (extra->health > 0) {
                extra->health -= SHOT_DAMAGE;
                if (extra->health <= 0) {
                    hit_mobj->sprite_index = ENEMY_DEFEATED_SPRITE;
                }
            }
        } else if (hit_mobj == player) {
            player_health -= SHOT_DAMAGE;
            if (player_health <= 0) {
                reset();
            }
        }
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