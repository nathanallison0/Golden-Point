#ifndef _GPOINT_PATHFIND_H_
#define _GPOINT_PATHFIND_H_

#include <SDL3Start.h>

float *pathfind(int start_x, int start_y, int end_x, int end_y, int *num_points);

bool pathfind_dist(float start_x, float start_y, float end_x, float end_y, float *dist);

#endif