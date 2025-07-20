#include "block.h"
#include <stdio.h>


M_Color hex_to_rgb(const char *hex) {
    if (!hex || hex[0] != '#' || strlen(hex) != 7)
        return (struct M_Color) {
            .r = 0.0,
            .g = 0.0,
            .b = 0.0
        };

    char rs[3], gs[3], bs[3];
    rs[0] = hex[1]; rs[1] = hex[2]; rs[2] = '\0';
    gs[0] = hex[3]; gs[1] = hex[4]; gs[2] = '\0';
    bs[0] = hex[5]; bs[1] = hex[6]; bs[2] = '\0';

    int ri = strtol(rs, NULL, 16);
    int gi = strtol(gs, NULL, 16);
    int bi = strtol(bs, NULL, 16);

    struct M_Color c;
    c.r = ri / 255.0;
    c.g = gi / 255.0;
    c.b = bi / 255.0;

    return c;
}


M_Color hsv_to_rgb(float h, float s, float v) {
    int i = (int)(h * 6);
    float f = h * 6 - i;
    float p = v * (1 - s);
    float q = v * (1 - f * s);
    float t = v * (1 - (1 - f) * s);

    float r, g, b;

    switch (i % 6) {
        case 0: r = v; g = t; b = p; break;
        case 1: r = q; g = v; b = p; break;
        case 2: r = p; g = v; b = t; break;
        case 3: r = p; g = q; b = v; break;
        case 4: r = t; g = p; b = v; break;
        case 5: r = v; g = p; b = q; break;
    }

    return (M_Color) {r, g, b};
}


void Mf_PrintBlock(M_TextBlock *block)
{
    printf("Block %p\n 1. %s\n 2. %s\n 3. %s\n 4. %s\n", block,
           block->name, block->text,
           block->text_color, block->background_color);
    fflush(stdout);
}


M_TextBlock* Mf_SearchBlock(M_TextBlock* list[], size_t list_size, const char* block_name)
{
    for (size_t i = 0; i < list_size; i++) {
        if (strcmp(((M_TextBlock*)list[i])->name, block_name) == 0) {
            return list[i];
        }
    }
    return NULL;
}


struct M_BlockInfo Mf_GetBlockInfo(cairo_t *cr, M_TextBlock *block)
{
    cairo_text_extents_t extents;
    cairo_font_extents_t fextents;
    if (block->text != NULL) {
        cairo_text_extents(cr, block->text, &extents);
        cairo_font_extents(cr, &fextents);
    }

    double text_height = fextents.ascent + fextents.descent;
    double text_width  = extents.width;

    return (struct M_BlockInfo) {
            .text_height  = text_height,
            .text_width   = text_width,
            .x_bearing = extents.x_bearing,
            .x_advance = extents.x_advance,
            .ascent  = fextents.ascent,
            .descent = fextents.descent
        };
}


void Mf_RenderBlock(cairo_t *cr, M_TextBlock *block,
                    int *position, int voffset,
                    struct M_BlockInfo *bstats)
{
    extern cairo_font_face_t* GLOBAL_FONT;

    int pos = *position;
    cairo_font_face_t* font;

    if (block->font)
        font = Mf_GetFont(block->font);
    else font = GLOBAL_FONT;

    cairo_set_font_face(cr, font);
    cairo_set_font_size(cr, 16);

    if (block->background_color) {
        M_Color b = hex_to_rgb(block->background_color);
        cairo_set_source_rgb(cr, b.r, b.g, b.b);
        cairo_rectangle(cr,
                        pos + bstats->x_bearing,
                        voffset - bstats->ascent,
                        bstats->text_width,
                        bstats->text_height);
        cairo_fill(cr);
    }

    if (block->text) {
        if (block->text_color) {
            M_Color t = hex_to_rgb(block->text_color);
            cairo_set_source_rgb(cr, t.r, t.g, t.b);
        } else {
            cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
        }

        cairo_move_to(cr, pos, voffset);
        cairo_show_text(cr, block->text);
    }

    int end_x = pos + bstats->x_advance;
    *position = end_x;
}
