#ifndef _GPOINT_CONSTANTS_H_
#define _GPOINT_CONSTANTS_H_

#define FALSE 0
#define TRUE 1

#if __EMSCRIPTEN__
#define WINDOW_SCALE 40
#else
#define WINDOW_SCALE 80
#endif

#define WINDOW_WIDTH (16 * WINDOW_SCALE)
#define WINDOW_HEIGHT (9 * WINDOW_SCALE)

#define FPS 60
#define FRAME_TARGET_TIME (1000 / FPS)

#define GRID_SPACING 64
#define GS GRID_SPACING
#define GRID_SPACING_2 (GS / 2)
#define WORLDSPACE ((float) GRID_SPACING / WINDOW_HEIGHT)

#define assert(x) if (!(x)) { printf("assertion error: " __FILE__ "\nline %d - " #x ": '%s'\n", __LINE__, SDL_GetError()); exit(1); }

#define fix_angle_360(angle) { if (angle < 0) angle += PI * 2; else if (angle >= PI * 2) angle -= PI * 2; }
#define fix_angle_180(angle) { if (angle < -PI) angle += PI * 2; else if (angle >= PI) angle -= PI * 2; }

#define range(left, sym1, var, sym2, right) (left sym1 var && var sym2 right)

#endif