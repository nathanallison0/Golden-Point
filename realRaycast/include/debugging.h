#ifndef _GPOINT_DEBUGGING_H_
#define _GPOINT_DEBUGGING_H_

#include <SDL3Start.h>

#define CHAR_VLS_LEN 11
#define FLT_VLS_LEN 3

enum {
    DG_WHITE,
    DG_YELLOW,
    DG_RED,
    DG_BLUE,
    DG_PURPLE,
    DG_GREEN,
    DG_BLACK
};
typedef Uint8 DG_Color;

typedef struct temp_dgp {
    float x;
    float y;
    DG_Color color_index;
} temp_dgp;

typedef struct fill_dgp {
    float x;
    float y;
    DG_Color color_index;
    struct fill_dgp *next;
} fill_dgp;

// Varlabels
typedef struct {
    char *label;
    Uint8 *value;
} char_varlabel;

typedef struct {
    char *label;
    float *value;
} flt_varlabel;

rgb get_dg_color_rgb(DG_Color c);

char *get_terminal_input_text(void);

void terminal_add_char(char c);

void terminal_pop_char(void);

void terminal_interpret_command(void);

void add_fill_dgp(float x, float y, DG_Color color_index, int *num_dgps, int max_dgps, struct fill_dgp **head, struct fill_dgp **tail);

void add_temp_dgp(float x, float y, DG_Color color_index, size_t *num_dgps, struct temp_dgp **list);

temp_dgp *get_temp_dgp(int index);

size_t get_num_temp_dgps(void);

void clear_temp_dgps(void);

fill_dgp *get_fill_dgp_head(void);

void init_debugging(void);

void debugging_end(void);

#endif