#ifndef _BLOCK_H_
#define _BLOCK_H_


#include <cairo/cairo.h>
#include <cairo/cairo-xlib.h>

#include <ft2build.h>
#include FT_FREETYPE_H
#include <fontconfig/fontconfig.h>
#include <stdio.h>

#include "font.h"



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
        *name,
        *text, *font,
        *text_color, *background_color,
        *command;

} M_TextBlock;


enum PenPos {
    START,
    CENTER,
    END,
};


M_Color hex_to_rgb(
    const char *hex
);


void Mf_PrintBlock(
    M_TextBlock *block
);


struct M_BlockInfo Mf_GetBlockInfo(
    cairo_t     *cr,
    M_TextBlock *block
);


M_TextBlock* Mf_SearchBlock(
    void         **list,
    size_t         list_size,
    const char    *block_name
);


void Mf_RenderSection(
    cairo_t     *cr,
    void        *section,
    size_t       list_size,
    enum PenPos  pp
);


void Mf_RenderBlock(
    cairo_t            *cr,
    M_TextBlock        *block,
    int                *position,
    int                 voffset,
    struct M_BlockInfo *bstats
);


#endif // _BLOCK_H_
