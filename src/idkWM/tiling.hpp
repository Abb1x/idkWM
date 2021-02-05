#ifndef TILING_H
#define TILING_H
#include <X11/X.h>
#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <X11/Xos.h>
#include <X11/Xresource.h>
#include <X11/Xutil.h>
#include <X11/cursorfont.h>
#include <X11/extensions/shape.h>
#include <X11/keysym.h>
#include <vector>
namespace idkWM
{
class tiling
{
public:
    bool enabled;
    void tile(const XMapRequestEvent event);
    static tiling *get();

private:
    Window master_window;
    std::vector<Window> window_list;
};
} // namespace idkWM

#endif
