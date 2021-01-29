#ifndef TYPES_H
#define TYPES_H
#include <X11/X.h>
#include <X11/Xlib.h>
namespace idkWM
{
struct frame_info
{
    bool created;
    bool resizable;
    bool movable;
    bool full_screen;

    Display *display;
    Window frame;

    int y, x, height, width;
};
struct position
{
    int x;
    int y;
};

} // namespace idkWM

#endif
