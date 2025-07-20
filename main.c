#include <X11/Xlib.h>
#include <X11/Xatom.h>

#include <cairo/cairo.h>
#include <cairo/cairo-xlib.h>

#include <ft2build.h>
#include <stddef.h>
#include FT_FREETYPE_H
#include <fontconfig/fontconfig.h>
#include <ini.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

#include "block.h"
#include "font.h"
#include "x11utils.h"
#include "config.h"


M_TextBlock* left_blocks[MAX_BLOCKS_IN_SECTION]   = {};
M_TextBlock* center_blocks[MAX_BLOCKS_IN_SECTION] = {};
M_TextBlock* right_blocks[MAX_BLOCKS_IN_SECTION]  = {};

size_t left_blocks_length  = 0;
size_t mid_blocks_length   = 0;
size_t right_blocks_length = 0;


Display*           display;
int                screen;
int                workspace;
cairo_font_face_t* GLOBAL_FONT;



int main(int argc, char** argv) {

    if (ini_parse("config", Mf_inihandler, NULL) < 0) {
        printf("Failed to load config.ini\n");
    }

    M_Color bg_color = hex_to_rgb(BAR_COLOR);
    GLOBAL_FONT = Mf_GetFont(BAR_FONT);

    display = XOpenDisplay(NULL);
    if (!display) {
        fprintf(stderr, "Failed to open X display\n");
        return 1;
    }

    screen = DefaultScreen(display);
    Window root = RootWindow(display, screen);

    int screen_width = DisplayWidth(display, screen);

    Window window = XCreateSimpleWindow(display, root, 0, 0,
                                        screen_width, BAR_HEIGHT,
                                        0, 0, 0);

    Atom wm_type = XInternAtom(display, "_NET_WM_WINDOW_TYPE", False);
    Atom wm_type_dock = XInternAtom(display, "_NET_WM_WINDOW_TYPE_DOCK", False);
    XChangeProperty(display, window, wm_type, XA_ATOM, 32,
                    PropModeReplace, (unsigned char*)&wm_type_dock, 1);

    Atom strut = XInternAtom(display, "_NET_WM_STRUT_PARTIAL", False);
    long strut_values[12] = {
        0, 0, BAR_HEIGHT, 0,   // left, right, top, bottom
        0, 0,                  // left_start_y, left_end_y
        0, 0,                  // right_start_y, right_end_y
        0, screen_width,       // top_start_x, top_end_x
        0, 0                   // bottom_start_x, bottom_end_x
    };

    XChangeProperty(display, window, strut, XA_CARDINAL, 32,
                    PropModeReplace, (unsigned char*)strut_values, 12);

    Atom strut_old = XInternAtom(display, "_NET_WM_STRUT", False);
    long strut_simple[4] = {0, 0, BAR_HEIGHT, 0};
    XChangeProperty(display, window, strut_old, XA_CARDINAL, 32,
                    PropModeReplace, (unsigned char*)strut_simple, 4);


    XSelectInput(display, RootWindow(display, DefaultScreen(display)), PropertyChangeMask);
    XMapWindow(display, window);

    cairo_surface_t* surface =
        cairo_xlib_surface_create(display, window,
                                  DefaultVisual(display, screen),
                                  screen_width, BAR_HEIGHT);

    cairo_t* cr = cairo_create(surface);

    cairo_xlib_surface_set_size(surface, screen_width, BAR_HEIGHT);
    XFlush(display);

    struct timespec last_time;
    clock_gettime(CLOCK_MONOTONIC, &last_time);

    Atom net_current_desktop = XInternAtom(display, "_NET_CURRENT_DESKTOP", False);

    XEvent event;
    while (1) {

        struct timespec now;
        clock_gettime(CLOCK_MONOTONIC, &now);

        double elapsed = (now.tv_sec - last_time.tv_sec) +
                         (now.tv_nsec - last_time.tv_nsec) / 1e9;

        if (elapsed >= _PROCESS_INTERVAL) {
            _process();
            last_time = now;
        }

        while (XPending(display)) {
            XNextEvent(display, &event);
            if (event.type == KeyPress)
               continue;
        }

        cairo_set_source_rgb(cr, bg_color.r, bg_color.g, bg_color.b);
        cairo_paint(cr);


        /**
         * Left Section Blocks Rendering
         * */
        if (left_blocks_length > 0) {
            int left_pen_pos = H_PADDING;
            for (size_t i = 0; i < left_blocks_length; i++) {
                struct M_BlockInfo bstats = Mf_GetBlockInfo(cr, left_blocks[i]);
                Mf_RenderBlock(cr, left_blocks[i],
                               &left_pen_pos, V_OFFSET,
                               &bstats);
            }
        }


        /**
         * Middle Section Blocks Rendering
         * */
        if (mid_blocks_length > 0) {
            // Render first block
            int mid_section_width = 0;
            for (size_t i = 0; i < mid_blocks_length; i++) {
                M_TextBlock* nblock = center_blocks[i];
                struct M_BlockInfo nbstats = Mf_GetBlockInfo(cr, nblock);
                mid_section_width += (int) nbstats.text_width;
            }

            // Render rest
            int mid_pen_pos = (screen_width / 2) - (mid_section_width / 2);
            struct M_BlockInfo mb1_stats = Mf_GetBlockInfo(cr, center_blocks[0]);
            Mf_RenderBlock(cr, center_blocks[0],
                           &mid_pen_pos, V_OFFSET,
                           &mb1_stats);

            for (size_t i = 1; i < mid_blocks_length; i++) {
                struct M_BlockInfo bstats = Mf_GetBlockInfo(cr, center_blocks[i]);
                if (i == mid_blocks_length - 1) {
                    Mf_RenderBlock(cr, center_blocks[i],
                                   &mid_pen_pos, V_OFFSET,
                                   &bstats);
                    break;
                }
                Mf_RenderBlock(cr, center_blocks[i],
                               &mid_pen_pos, V_OFFSET,
                               &bstats);
            }
        }


        /**
         * Right Section Blocks Rendering
         * */
        if (right_blocks_length > 0) {
            // Render first block
            int right_section_width = 0;
            for (size_t i = 0; i < right_blocks_length; i++) {
                M_TextBlock* nblock = right_blocks[i];
                struct M_BlockInfo nbstats = Mf_GetBlockInfo(cr, nblock);
                right_section_width += (int) nbstats.text_width;
            }

            // Render rest
            int right_pen_pos = screen_width - right_section_width - H_PADDING;
            struct M_BlockInfo rb1_stats = Mf_GetBlockInfo(cr, right_blocks[0]);
            Mf_RenderBlock(cr, right_blocks[0],
                           &right_pen_pos, V_OFFSET,
                           &rb1_stats);

            for (size_t i = 1; i < right_blocks_length; i++) {
                struct M_BlockInfo bstats = Mf_GetBlockInfo(cr, right_blocks[i]);
                if (i == right_blocks_length - 1) {
                    Mf_RenderBlock(cr, right_blocks[i],
                                   &right_pen_pos, V_OFFSET,
                                   &bstats);
                    break;
                }
                Mf_RenderBlock(cr, right_blocks[i],
                               &right_pen_pos, V_OFFSET,
                               &bstats);
            }
        }


        usleep(16666);
        XFlush(display);
    }

    cairo_destroy(cr);
    cairo_surface_destroy(surface);
    XDestroyWindow(display, window);
    XCloseDisplay(display);
    return 0;
}

