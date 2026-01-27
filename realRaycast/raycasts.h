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

#define c_hx        v->c_hx
#define c_hy        v->c_hy
#define d_hx        v->d_hx
#define d_hy        v->d_hy
#define c_vx        v->c_vx
#define c_vy        v->c_vy
#define d_vx        v->d_vx
#define d_vy        v->d_vy
#define c_epsilon_h v->c_epsilon_h
#define d_epsilon_h v->d_epsilon_h
#define c_epsilon_v v->c_epsilon_v
#define d_epsilon_v v->d_epsilon_v
#define quadrant v->quadrant
void raycast_vars(float x, float y, float angle, raycast_info *v) {
    quadrant = (char) ((angle < 0 ? angle + (PI * 2) : angle) / PI_2) + 1;
    
    // Limit alpha to first quadrant
    float alpha = angle - (quadrant == 1 ? 0 : (quadrant == 2 ? PI_2 : (quadrant == 3 ? PI : PI + PI_2)));
    float tan_alpha = tanf(alpha);
    if (quadrant == 1) {
        alpha = angle;
        c_hy = ceilf(y / GRID_SPACING);
        c_hx = ((c_hy * GRID_SPACING - y) / tan_alpha) + x;
        d_hx = GRID_SPACING / tan_alpha;
        d_hy = 1;
        c_vx = (int) (x / GRID_SPACING) + 1;
        c_vy = (tan_alpha * ((c_vx * GRID_SPACING) - x)) + y;
        d_vx = 1;
        d_vy = tan_alpha * (d_vx * GRID_SPACING);
    } else if (quadrant == 2) {
        alpha = angle - (PI_2);
        c_hy = ceilf(y / GRID_SPACING);
        c_hx = tan_alpha * (y - (c_hy * GRID_SPACING)) + x;
        d_hy = 1;
        d_hx = -GRID_SPACING * tan(alpha);
        c_vx = (int) (x / GRID_SPACING);
        c_vy = y + ((x - (c_vx * GRID_SPACING)) / tan_alpha);
        d_vx = -1;
        d_vy = GRID_SPACING / tan_alpha;
    } else if (quadrant == 3) {
        alpha = angle - PI;
        c_hy = (int) (y / GRID_SPACING);
        c_hx = x + ((c_hy * GRID_SPACING) - y) / tan_alpha;
        d_hy = -1;
        d_hx = -GRID_SPACING * tanf((PI_2) - alpha);
        c_vx = (int) (x / GRID_SPACING);
        c_vy = y - (tan_alpha * (x - (c_vx * GRID_SPACING)));
        d_vx = -1;
        d_vy = -GRID_SPACING * tan_alpha;
    } else {
        alpha = angle - (PI + (PI_2));
        c_hy = (int) (y / GRID_SPACING);
        c_hx = x - tan_alpha * ((c_hy * GRID_SPACING) - y);
        d_hy = -1;
        d_hx = GRID_SPACING * tan(alpha);
        c_vx = (int) (x / GRID_SPACING) + 1;
        c_vy = y - ((c_vx * GRID_SPACING) - x) / tan_alpha;
        d_vx = 1;
        d_vy = -GRID_SPACING / tan_alpha;
    }

    float diagonal_dist = fabsf((PI_4) - alpha) / (PI * 20);
    c_epsilon_h = (sqrtf(powf(c_hx - x, 2) + powf((c_hy * GRID_SPACING) - y, 2)) / GRID_SPACING) * diagonal_dist;
    d_epsilon_h = (sqrtf(powf(d_hx, 2) +     powf(d_hy * GRID_SPACING, 2))       / GRID_SPACING) * diagonal_dist;
    c_epsilon_v = (sqrtf(powf((c_vx * GRID_SPACING) - x, 2) + powf(c_vy - y, 2)) / GRID_SPACING) * diagonal_dist;
    d_epsilon_v = (sqrtf(powf(d_vx * GRID_SPACING, 2) +       powf(d_vy, 2))     / GRID_SPACING) * diagonal_dist;

    v->door_hit.door = NULL;
    v->texture_offset = 0;
}

#define bounds_x(x) (0 < (x) && (x) < GRID_WIDTH * GRID_SPACING)
#define bounds_x_int(x) (0 < (x) && (x) < GRID_WIDTH)
#define bounds_y(y) (0 < (y) && (y) < GRID_HEIGHT * GRID_SPACING)
#define bounds_y_int(y) (0 < (y) && (y) < GRID_HEIGHT)

Uint8 step_ray_h(raycast_info *v) {
    if (show_player_vision) {
        temp_dgp(c_hx, c_hy * GRID_SPACING, DG_BLUE);
    }

    float rel_wall_hit_h = fmodf(c_hx, GRID_SPACING);
    if (!bounds_x(c_hx) || !bounds_y_int(c_hy)) {
        return 0;
    }

    Uint8 left_down =  get_map((c_hx - (c_epsilon_h / 2)) / GRID_SPACING, c_hy - 1);
    Uint8 right_down = get_map((c_hx + (c_epsilon_h / 2)) / GRID_SPACING, c_hy - 1);
    Uint8 left_up =    get_map((c_hx - (c_epsilon_h / 2)) / GRID_SPACING, c_hy);
    Uint8 right_up =   get_map((c_hx + (c_epsilon_h / 2)) / GRID_SPACING, c_hy);

    Uint8 hit;
    if ((left_down || left_up) && (right_down || right_up)) {
        if (rel_wall_hit_h < GRID_SPACING / 2) {
            goto choose_from_left;
        } else {
            goto choose_from_right;
        }
    } else if (left_down || left_up) {
        goto choose_from_left;
    } else {
        goto choose_from_right;
    }

    choose_from_left:
    if (left_down && left_up) {
        if (quadrant == 1 || quadrant == 2) {
            hit = left_down;
        } else {
            hit = left_up;
        }
    } else if (left_down) {
        hit = left_down;
    } else {
        hit = left_up;
    }
    goto process_hit;

    choose_from_right:
    if (right_down && right_up) {
        if (quadrant == 1 || quadrant == 2) {
            hit = right_down;
        } else {
            hit = right_up;
        }
    } else if (right_down) {
        hit = right_down;
    } else {
        hit = right_up;
    }

    process_hit:
    if (hit == TILE_SOLID) {
        return 0;
    }
    
    if (hit == TILE_HORIZ_DOOR) {
        door *door;

        if (quadrant == 1 || quadrant == 2) {
            door = get_door_coords(c_hx, (c_hy * GRID_SPACING) + (GRID_SPACING / 2));
        } else {
            door = get_door_coords(c_hx, (c_hy * GRID_SPACING) - (GRID_SPACING / 2));
        }

        // Record this door as the one passed if we haven't passed one already
        if (door && !v->door_hit.door) {
            v->door_hit.door = door;
            v->door_hit.x = c_hx + (d_hx / 2);
            v->door_hit.y = (c_hy + (d_hy * 0.5f)) * GRID_SPACING;
        }

        // If we are at a door and either the door doesn't open or it does and we have hit it,
        // register a hit halfway through the space
        rel_wall_hit_h += d_hx / 2;
        if (0 <= rel_wall_hit_h && rel_wall_hit_h < GRID_SPACING && (!door || rel_wall_hit_h <= door->progress)) {
            if (door) {
                v->texture_offset = GRID_SPACING - door->progress;
            }
            return 0;
        }

        // If we didn't hit it but will again on the next iteration, increment extra
        // to not hit the back side of the door space
        float next_rel_hit_x = rel_wall_hit_h + (d_hx / 2);
        if (0 <= next_rel_hit_x && next_rel_hit_x < GRID_SPACING) {
            c_hx += d_hx; c_hy += d_hy;
        }
    }

    c_hx += d_hx; c_hy += d_hy;
    c_epsilon_h += d_epsilon_h;
    return 1;
}

Uint8 step_ray_v(raycast_info *v) {
    if (show_player_vision) {
        temp_dgp(c_vx * GRID_SPACING, c_vy, DG_GREEN);
    }
    if (!(
        (bounds_x_int(c_vx) && bounds_y(c_vy)) && !(
            ( get_map(c_vx - 1, (c_vy - (c_epsilon_v / 2)) / GRID_SPACING) || get_map(c_vx - 1, (c_vy + (c_epsilon_v / 2)) / GRID_SPACING) ) ||
            ( get_map(c_vx, (c_vy - (c_epsilon_v / 2)) / GRID_SPACING) || get_map(c_vx, (c_vy + (c_epsilon_v / 2)) / GRID_SPACING) )
        )
    )) {
        return 0;
    }
    
    c_vx += d_vx; c_vy += d_vy;
    c_epsilon_v += d_epsilon_v;
    return 1;
}

xy raycast(float x, float y, float angle, raycast_info *v, int *texture_index, int *texture_col) {
    raycast_vars(x, y, angle, v);
    while (step_ray_h(v));
    while (step_ray_v(v));
    
    if (point_dist(c_hx, c_hy * GRID_SPACING, x, y) < point_dist(c_vx * GRID_SPACING, c_vy, x, y)) {
        if (texture_index) {
            *texture_index = /* get_horiz_texture(c_hx, c_hy); */ horiz_textures[c_hy][(int) (c_hx / GRID_SPACING)];
        }

        if (texture_col) {
            *texture_col = ((fmodf(c_hx, GRID_SPACING) + v->texture_offset) / GRID_SPACING) * TEXTURE_WIDTH;

            // Account for drawing reverse for backward-facing walls
            if (quadrant == 1 || quadrant == 2) {
                *texture_col = TEXTURE_WIDTH - *texture_col - 1;
            }
        }

        // If hit door, return coordinates in the middle of the space
        if (v->door_hit.door) {
            c_hx += d_hx / 2;
            c_hy += d_hy / 2;
            return (xy) {c_hx + (d_hx / 2), (c_hy + (d_hy * 0.5f)) * GRID_SPACING};
        }
        return (xy) {c_hx, c_hy * GRID_SPACING};
    } else {
        if (texture_index) {
            *texture_index = /* get_vert_texture(c_vx, c_vy); */ vertical_textures[(int) c_vy / GRID_SPACING][c_vx];
        }
        if (texture_col) {
            *texture_col = (fmodf(c_vy, GRID_SPACING) / GRID_SPACING) * TEXTURE_WIDTH;

            // Account for drawing reverse for backward-facing walls
            if (quadrant == 2 || quadrant == 3) {
                *texture_col = TEXTURE_WIDTH - *texture_col - 1;
            }
        }

        return (xy) {c_vx * GRID_SPACING, c_vy};
    }
}

#undef c_hx
#undef c_hy
#undef d_hx
#undef d_hy
#undef c_vx
#undef c_vy
#undef d_vx
#undef d_vy
#undef c_epsilon_h
#undef d_epsilon_h
#undef c_epsilon_v
#undef d_epsilon_v
#undef quadrant

Uint8 raycast_to_h(int y_to, raycast_info *v) {
    Uint8 success;
    while (TRUE) {
        success = step_ray_h(v);
        if (((v->quadrant == 1 || v->quadrant == 2) && v->c_hy >= y_to) ||
            ((v->quadrant == 3 || v->quadrant == 4) && v->c_hy <= y_to)) {
                return TRUE;
            }
        if (!success) {
            return FALSE;
        }
    }
}

Uint8 raycast_to_v(int x_to, raycast_info *v) {
    Uint8 success;
    while (TRUE) {
        success = step_ray_v(v);
        if (((v->quadrant == 1 || v->quadrant == 4) && v->c_vx >= x_to) ||
            ((v->quadrant == 2 || v->quadrant == 3) && v->c_vx <= x_to)) {
                return TRUE;
            }
        if (!success) {
            return FALSE;
        }
    }
}

enum {
    RAY_NOHIT,
    RAY_HORIZHIT,
    RAY_VERTHIT
};
typedef Uint8 ray_hit;

ray_hit raycast_to_x(float x1, float y1, float x2, float angle, raycast_info *v) {
    float y2 = y1 + (tanf(angle) * (x2 - x1));
    raycast_vars(x1, y1, angle, v);

    int x_to;
    if (v->quadrant == 1 || v->quadrant == 4) {
        x_to = ceilf(x2 / GRID_SPACING);
    } else {
        x_to = (int) (x2 / GRID_SPACING);
    }

    int y_to;
    if (v->quadrant == 1 || v->quadrant == 2) {
        y_to = ceilf(y2 / GRID_SPACING);
    } else {
        y_to = (int) (y2 / GRID_SPACING);
    }

    Uint8 horiz_success = raycast_to_h(y_to, v);
    Uint8 vert_success = raycast_to_v(x_to, v);

    if (horiz_success && vert_success) {
        return RAY_NOHIT;
    }

    if (horiz_success == vert_success) {
        // If both failed, figure out which hit was closer
        if (point_dist(x1, y1, v->c_hx, v->c_hy * GRID_SPACING) < point_dist(x1, y1, v->c_vx * GRID_SPACING, v->c_vy)) {
            return RAY_HORIZHIT;
        }
        return RAY_VERTHIT;
    }

    if (horiz_success) {
        return RAY_VERTHIT;
    }
    return RAY_HORIZHIT;
}