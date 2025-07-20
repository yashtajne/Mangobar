#ifndef _X11UTILS_H_
#define _X11UTILS_H_

#include <X11/Xlib.h>
#include <stdio.h>
#include <X11/Xatom.h>

int get_current_desktop(Display *dpy);

#endif // _X11UTILS_H_
