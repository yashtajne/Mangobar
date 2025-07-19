

#include <X11/Xlib.h>
#include <X11/Xatom.h>

#include <cairo/cairo.h>
#include <cairo/cairo-xlib.h>

#include <ft2build.h>
#include FT_FREETYPE_H
#include <fontconfig/fontconfig.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

#include "block.h"
#include "font.h"
#include "command.h"


#define _PROCESS_INTERVAL 1.0 // in seconds

static int   BAR_HEIGHT = 24;
static int   BAR_OFFSET = 0;
static int   V_OFFSET   = 18;
static int   H_PADDING  = 5;
static char* BAR_COLOR  = "#1f1f28";
static char* BAR_FONT   = "Cascadia CodeNF:extrabold";

cairo_font_face_t* GLOBAL_FONT;

void* left_blocks[1]   = {};
void* center_blocks[1] = {};
void* right_blocks[3]  = {};
// size_t positions[sizeof(blocks) / sizeof(blocks[0])];

Display* display;

int get_current_desktop(Display *dpy) {
    Atom actual_type;
    int actual_format;
    unsigned long nitems, bytes_after;
    unsigned char *prop = NULL;

    Atom net_current_desktop = XInternAtom(dpy, "_NET_CURRENT_DESKTOP", True);
    if (net_current_desktop == None) {
        fprintf(stderr, "_NET_CURRENT_DESKTOP not supported by WM\n");
        return -1;
    }

    int status = XGetWindowProperty(dpy, DefaultRootWindow(dpy),
                                    net_current_desktop,
                                    0, 1, False, XA_CARDINAL,
                                    &actual_type, &actual_format,
                                    &nitems, &bytes_after, &prop);

    if (status != Success || !prop) {
        fprintf(stderr, "Failed to get _NET_CURRENT_DESKTOP\n");
        return -1;
    }

    int desktop = *(unsigned long *)prop;
    XFree(prop);

    return desktop;
}


void _process()
{
    M_TextBlock* datetime_block = right_blocks[2];
    char* curr_datetime = Mf_RunCommand("date");
    datetime_block->text = curr_datetime;

    M_TextBlock* volume_block = right_blocks[1];
    char* curr_volume = Mf_RunCommand("amixer get Master | awk -F'[][]' '/Left:/ { print $2; exit }'");
    char vol_buffer[32];
    snprintf(vol_buffer, sizeof(vol_buffer), "vol %s   ", curr_volume);
    volume_block->text = strdup(vol_buffer);

    M_TextBlock* brightness_block = right_blocks[0];
    char* curr_brightness = Mf_RunCommand("brightnessctl | awk -F '[()%]' '/Current/ { print $2 }'");
    char bgr_buffer[32];
    snprintf(bgr_buffer, sizeof(bgr_buffer), "bri %s%%   ", curr_brightness);
    brightness_block->text = strdup(bgr_buffer);
}


int main(int argc, char** argv) {

    left_blocks[0] = &(M_TextBlock) {
        .text = "Hello Mangobar!",
    };

    center_blocks[0] = &(M_TextBlock) {
        .text = "[ 1 ]",
    };

    right_blocks[0] = &(M_TextBlock) {
        .text = "32 aug 2029 10:69:10 AM",
    };

    right_blocks[1] = &(M_TextBlock) {
        .text = "32 aug 2029 10:69:10 AM",
    };

    right_blocks[2] = &(M_TextBlock) {
        .text = "32 aug 2029 10:69:10 AM",
    };


    M_Color bg_color = hex_to_rgb(BAR_COLOR);
    GLOBAL_FONT = Mf_GetFont(BAR_FONT);

    display = XOpenDisplay(NULL);
    if (!display) {
        fprintf(stderr, "Failed to open X display\n");
        return 1;
    }

    int screen = DefaultScreen(display);
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

    static float hue = 0.0f;
    hue += 0.01f;
    if (hue > 1.0f) hue = 0.0f;

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

            if (event.type == PropertyNotify) {
                if (event.xproperty.atom == net_current_desktop) {
                    int ws = get_current_desktop(display);
                    M_TextBlock* workspace_block = center_blocks[0];
                    char buffer[32];
                    snprintf(buffer, sizeof(buffer), "[ %d ]", ws+1);
                    workspace_block->text = strdup(buffer);
                }
            }
        }

        // hue += 0.01f;
        // if (hue > 1.0f) hue = 0.0f;
        // float r = hue, g = 1.0 - hue, b = 0.5f;
        // cairo_set_source_rgb(cr, r, g, b);

        cairo_set_source_rgb(cr, bg_color.r, bg_color.g, bg_color.b);
        cairo_paint(cr);

        /**
         * Left Section Blocks Rendering
         * */
        int left_blocks_length = 1;
        int left_next_space = H_PADDING;
        for (size_t i = 0; i < left_blocks_length; i++) {
            struct M_BlockInfo bstats = Mf_GetBlockStats(cr, left_blocks[i]);
            Mf_RenderBlock(cr, left_blocks[i],
                           &left_next_space, V_OFFSET,
                           &bstats, LEFT);
        }


        /**
         * Middle Section Blocks Rendering
         * */
        int mid_blocks_length = 1;
        if (mid_blocks_length > 0) {
            // Render first block
            int mid_section_width = 0;
            for (size_t i = 0; i < mid_blocks_length; i++) {
                M_TextBlock* nblock = center_blocks[i];
                struct M_BlockInfo nbstats = Mf_GetBlockStats(cr, nblock);
                mid_section_width += (int) nbstats.text_width;
            }

            // Render rest
            int mid_pen_pos = (screen_width / 2) - (mid_section_width / 2);
            struct M_BlockInfo mb1_stats = Mf_GetBlockStats(cr, center_blocks[0]);
            Mf_RenderBlock(cr, center_blocks[0],
                           &mid_pen_pos, V_OFFSET,
                           &mb1_stats, CENTER);

            for (size_t i = 1; i < mid_blocks_length; i++) {
                struct M_BlockInfo bstats = Mf_GetBlockStats(cr, center_blocks[i]);
                if (i == mid_blocks_length - 1) {
                    Mf_RenderBlock(cr, center_blocks[i],
                                   &mid_pen_pos, V_OFFSET,
                                   &bstats, CENTER);
                    break;
                }
                Mf_RenderBlock(cr, center_blocks[i],
                               &mid_pen_pos, V_OFFSET,
                               &bstats, CENTER);
            }
        }


        /**
         * Right Section Blocks Rendering
         * */
        int right_blocks_length = 3;
        if (right_blocks_length > 0) {
            // Render first block
            int right_section_width = 0;
            for (size_t i = 0; i < right_blocks_length; i++) {
                M_TextBlock* nblock = right_blocks[i];
                struct M_BlockInfo nbstats = Mf_GetBlockStats(cr, nblock);
                right_section_width += (int) nbstats.text_width;
            }

            // Render rest
            int right_pen_pos = screen_width - right_section_width - H_PADDING;
            struct M_BlockInfo rb1_stats = Mf_GetBlockStats(cr, right_blocks[0]);
            Mf_RenderBlock(cr, right_blocks[0],
                           &right_pen_pos, V_OFFSET,
                           &rb1_stats, CENTER);

            for (size_t i = 1; i < right_blocks_length; i++) {
                struct M_BlockInfo bstats = Mf_GetBlockStats(cr, right_blocks[i]);
                if (i == right_blocks_length - 1) {
                    Mf_RenderBlock(cr, right_blocks[i],
                                   &right_pen_pos, V_OFFSET,
                                   &bstats, CENTER);
                    break;
                }
                Mf_RenderBlock(cr, right_blocks[i],
                               &right_pen_pos, V_OFFSET,
                               &bstats, CENTER);
            }
        }


        usleep(16000);
        // usleep(1000000);  // 1 second = 1,000,000 microseconds
        XFlush(display);
    }

    cairo_destroy(cr);
    cairo_surface_destroy(surface);
    XDestroyWindow(display, window);
    XCloseDisplay(display);
    return 0;
}

