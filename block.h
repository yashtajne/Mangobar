#ifndef _BLOCK_H_
#define _BLOCK_H_


#include <cairo/cairo.h>
#include <cairo/cairo-xlib.h>

#include <ft2build.h>
#include FT_FREETYPE_H
#include <fontconfig/fontconfig.h>

#include "font.h"


enum Alignment {
    LEFT,
    CENTER,
    RIGHT,
};

struct M_BlockInfo {

    double
        text_height, text_width,
        x_advance, x_bearing,
        ascent, descent;

};

typedef struct M_Color {

    double r, g, b;

} M_Color;


typedef struct M_TextBlock {

    char
        *text, *font,
        *text_color, *background_color;

    enum Alignment place_self;

} M_TextBlock;


struct M_BlockInfo Mf_GetBlockStats(cairo_t *cr, void *block);

void Mf_RenderBlock(cairo_t *cr, M_TextBlock *block,
                    int *position, int voffset,
                    struct M_BlockInfo *bstats,
                    enum Alignment align);

M_Color hex_to_rgb(const char *hex);

#endif // _BLOCK_H_
