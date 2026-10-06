#ifndef _GPOINT_BASICFONT_H_
#define _GPOINT_BASICFONT_H_

#include "../../../SDL/start/include/SDL3Start.h"
#include "../include/BasicFont.h"

#define BF_CHAR_WIDTH 5
#define BF_CHAR_HEIGHT 5

typedef struct {
    char* text;
    size_t len;
    int alloc;
} alstring;

int BF_GetCharIndex(char character);

void BF_SetTextAttrs(int x, int y, int font_size, rgba color);

void BF_Newline(void);

void BF_DrawChar(char c);

void BF_FillText(char *text, Uint8 font_size, int wrap_length, rgba font_color, bool draw_cursor);

void BF_DrawText(char *text, int x, int y, int font_size, int wrap_length, rgba font_color, bool show_cursor);

int BF_DrawChars(char *text, int amount);

alstring *BF_WriteTextWordWrap(char *text, int font_size, int wrap_length, int *num_lines);

alstring *alstring_init(int alloc);

void alstring_append(alstring *str, char c);

size_t alstring_appendsub(alstring *a, char *text, int length);

void alstring_pop(alstring *str);

void alstring_clear(alstring *str);

void alstring_destroy(alstring *str);

#endif