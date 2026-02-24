#include <math.h>

enum {
    PSDIR_RIGHT_UP,
    PSDIR_RIGHT_DOWN,
    PSDIR_RIGHT,
    PSDIR_LEFT_UP,
    PSDIR_LEFT_DOWN,
    PSDIR_LEFT,
    PSDIR_UP,
    PSDIR_DOWN,
    PS_NUM_DIRS
};
typedef Uint8 ps_dir;

#define ps_dir_to_x(dir) (dir < PSDIR_LEFT_UP ? 1 : (dir < PSDIR_UP ? -1 : 0))
#define ps_dir_to_y(dir) (dir % 3 == 0 ? 1 : (dir % 3 == 1 ? -1 : 0))

__doubly_linked_list_init__(
    p_space,
        int x; int y;
        struct p_space *path_prev;
        float dist_to;
        float dist_total;
)

__doubly_linked_list_creator__(
    p_space,
    (int x, int y, p_space *path_prev, float dist_to, float dist_total),
        item->x = x;
        item->y = y;
        item->path_prev = path_prev;
        item->dist_to = dist_to;
        item->dist_total = dist_total;
)

__linked_list_destroy_all_head__(p_space)

p_space *p_open_head;
p_space *p_closed_head;
int p_end_x, p_end_y;
p_space *p_current;

// Manhattan distance
/* int p_heuristic(int start_x, int start_y) {
    return abs(start_x - p_end_x) + abs(start_y - p_end_y);
} */

// Octile distance
float p_heuristic(int start_x, int start_y) {
    float dx = abs(start_x - p_end_x);
    float dy = abs(start_y - p_end_y);

    return dx + dy + (((float) M_SQRT2 - 2) * fminf(dx, dy));
}

void p_detach_open(p_space *s) {
    if (s->prev) {
        s->prev->next = s->next;
    } else {
        p_open_head = s->next;
    }

    if (s->next) {
        s->next->prev = s->prev;
    }
}

void p_insert_sort_open(p_space *s) {
    // Sort by closest to farthest
    if (p_open_head) {
        if (p_open_head->dist_total > s->dist_total) {
            // This is the shortest open spaces, add at head
            s->next = p_open_head;
            p_open_head->prev = s;
            p_open_head = s;
            s->prev = NULL;
        } else {
            // Find last closer space
            p_space *closer = p_open_head;
            while (closer->next && closer->next->dist_total < s->dist_total) {
                closer = closer->next;
            }

            // Insert this space after the closer space
            p_space *farther = closer->next;
            closer->next = s;
            if (farther) {
                farther->prev = s;
            }
            s->next = farther;
            s->prev = closer;
        }
    } else {
        // There are no other spaces, put at head
        s->next = NULL;
        s->prev = NULL;
        p_open_head = s;
    }
}

bool p_handle_adj(ps_dir dir_from_src) {
    int new_x = p_current->x + ps_dir_to_x(dir_from_src);
    int new_y = p_current->y + ps_dir_to_y(dir_from_src);

    if (new_x == p_end_x && new_y == p_end_y) {
        return TRUE;
    }


    // Don't pathfind through walls or out of bounds
    if (
        new_x >= 0 && new_x < GRID_WIDTH &&
        new_y >= 0 && new_y < GRID_HEIGHT &&
        !get_map(new_x, new_y)
    ) {
        // Don't travel diagonally if there is a wall in the way
        if (
            ((dir_from_src == PSDIR_RIGHT_DOWN || dir_from_src == PSDIR_RIGHT_UP) &&
            get_map(p_current->x + 1, p_current->y)) ||

            ((dir_from_src == PSDIR_LEFT_UP || dir_from_src == PSDIR_RIGHT_UP) &&
            get_map(p_current->x, p_current->y + 1)) ||

            ((dir_from_src == PSDIR_LEFT_DOWN || dir_from_src == PSDIR_LEFT_UP) &&
            get_map(p_current->x - 1, p_current->y)) ||

            ((dir_from_src == PSDIR_LEFT_DOWN || dir_from_src == PSDIR_RIGHT_DOWN) &&
            get_map(p_current->x, p_current->y - 1))
        ) {
            return FALSE;
        }

        // Get the dist to travel from the last space to this one
        // If diagonal, sqrt 2
        // else, 1
        float step_dist = 
            (dir_from_src < PSDIR_LEFT && dir_from_src != PSDIR_RIGHT) ? 
                (float) M_SQRT2 :
                1.0f;

        // Calculate the distance of this new space
        // (dist to get here plus estimated remaining dist)
        float this_dist_to = p_current->dist_to + step_dist;
        float this_dist_total = this_dist_to + p_heuristic(new_x, new_y);

        // Check if there is already an open space here
        for (p_space *s = p_open_head; s; s = s->next) {
            if (s->x == new_x && s->y == new_y) {
                // If our distance is shorter, replace with our variables
                if (s->dist_total > this_dist_total) {
                    s->dist_to = this_dist_to;
                    s->dist_total = this_dist_total;
                    s->path_prev = p_current;

                    // Re-sort this space
                    p_detach_open(s);
                    p_insert_sort_open(s);
                    // If the prev space is greater than us, move back
                    /* while (s->prev && s->prev->dist_total > this_dist_total) {
                        s->prev->next = s->next;
                        if (s->next) {
                            s->next->prev = s->prev;
                        }

                        s->next = s->prev;
                        if (s->next) {
                            s->next->prev = s;
                        }
                        
                        s->prev = s->prev->prev;
                        if (s->prev) {
                            s->prev->next = s;
                        }
                    }

                    // If the next space is lesser, move forward
                    while (s->next && s->next->dist_total < this_dist_total) {
                        s->next->prev = s->prev;
                        if (s->prev) {
                            s->prev->next = s->next;
                        }

                        s->next = s->next->next;
                        s->prev = s->next;
                        s->prev->next = s;
                        if (s->next) {
                            s->next->prev = s;
                        }
                    } */
                }

                return FALSE;
            }
        }

        // Check if there is already a closed space here
        for (p_space *s = p_closed_head; s; s = s->next) {
            if (s->x == new_x && s->y == new_y) {
                // If our distance is shorter, replace with our variables
                // and mark space as open
                if (s->dist_total > this_dist_total) {
                    s->dist_to = this_dist_to;
                    s->dist_total = this_dist_total;
                    s->path_prev = p_current;

                    if (s->prev) {
                        s->prev->next = s->next;
                    } else {
                        p_closed_head = s->next;
                    }

                    if (s->next) {
                        s->next->prev = s->prev;
                    }

                    p_insert_sort_open(s);
                }
                return FALSE;
            }
        }

        // We have not considered this space before, so mark it as open
        p_space *new_space = p_space_create(
            new_x, new_y,
            p_current,
            this_dist_to,
            this_dist_total
        );
        
        p_insert_sort_open(new_space);
    }

    return FALSE;
}

float *pathfind(int start_x, int start_y, int end_x, int end_y, int *num_points) {
    // Don't pathfind out of bounds
    if (
        start_x < 0 || start_x >= GRID_WIDTH || start_y < 0 || start_y >= GRID_HEIGHT ||
        end_x < 0 || end_x >= GRID_WIDTH || end_y < 0 || end_y >= GRID_HEIGHT
    ) {
        return NULL;
    }

    p_end_x = end_x;
    p_end_y = end_y;
    p_closed_head = NULL;
    p_open_head = p_space_create(
        start_x, start_y,
        NULL, // not applicable
        0, p_heuristic(start_x, start_y)
    );

    // Close all open spaces until goal is reached
    ps_dir try_dir;
    while (TRUE) {
        // Consider the space with the least distance
        // (sorted closest to farthest)
        p_current = p_open_head;

        // Calculate each adjacent space, checking if we reached the goal
        for (try_dir = 0; try_dir < PS_NUM_DIRS; try_dir++) {
            if (p_handle_adj(try_dir)) {
                goto goal;
            }
        }

        // Remove space from open list
        p_detach_open(p_current);

        // Add space to closed list
        if (p_closed_head) {
            p_closed_head->prev = p_current;
            p_current->next = p_closed_head;
            p_closed_head = p_current;
        } else {
            p_closed_head = p_current;
            p_closed_head->next = NULL;
        }
        p_closed_head->prev = NULL;

        // If all spaces are closed, we failed to find a path
        if (p_open_head == NULL) {
            p_space_destroy_all(p_open_head);
            p_space_destroy_all(p_closed_head);
            *num_points = 0;
            return NULL;
        }
    }

    // Get rid of compiler warning: declaration before label
    float *points;
    goal:
    points = malloc(sizeof(float) * 2);

    // Construct point path backwards in memory
    points[0] = (p_end_x * GRID_SPACING) + (GRID_SPACING / 2);
    points[1] = (p_end_y * GRID_SPACING) + (GRID_SPACING / 2);

    int i = 1;
    // Include starting space
    for (p_space *s = p_current; s; s = s->path_prev) {
        points = realloc(points, (i + 1) * (sizeof(float) * 2));
        points[i * 2] = (s->x * GRID_SPACING) + (GRID_SPACING / 2);
        points[(i * 2) + 1] = (s->y * GRID_SPACING) + (GRID_SPACING / 2);
        i++;
    }

    *num_points = i;
    p_space_destroy_all(p_open_head);
    p_space_destroy_all(p_closed_head);
    return points;
}