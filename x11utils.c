#include "x11utils.h"


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

