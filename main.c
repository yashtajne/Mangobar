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
#include "config.h"



void* left_blocks[MAX_BLOCKS_IN_SECTION]   = {};
void* center_blocks[MAX_BLOCKS_IN_SECTION] = {};
void* right_blocks[MAX_BLOCKS_IN_SECTION]  = {};

size_t left_blocks_length  = 0;
size_t mid_blocks_length   = 0;
size_t right_blocks_length = 0;

void** command_blocks;
size_t command_blocks_length = 20;


Display*           display;
int                screen;
int                screen_width;
int                workspace;
cairo_font_face_t* GLOBAL_FONT;


int V_OFFSET  = 18;
int H_PADDING = 5;

int main(int argc, char** argv) {

    command_blocks = (void**)malloc(command_blocks_length * sizeof(void*));

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
    screen_width = DisplayWidth(display, screen);
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
        Mf_RenderSection(cr, left_blocks, left_blocks_length,
                         START);

        /**
         * Middle Section Blocks Rendering
         * */
        Mf_RenderSection(cr, center_blocks, mid_blocks_length,
                         CENTER);

        /**
         * Right Section Blocks Rendering
         * */
        Mf_RenderSection(cr, right_blocks,
                         right_blocks_length,
                         END);

        usleep(16666);
        XFlush(display);
    }

    cairo_destroy(cr);
    cairo_surface_destroy(surface);
    XDestroyWindow(display, window);
    XCloseDisplay(display);
    return 0;
}

