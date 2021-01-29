#ifndef IDKWM_H
#define IDKWM_H
#include "types.hpp"
#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xos.h>
#include <X11/Xresource.h>
#include <X11/Xutil.h>
#include <X11/extensions/shape.h>
#include <X11/keysym.h>
#include <unordered_map>
namespace idkWM
{
#define JSON_GET(x) x.substr(1, x.size() - 2);
class wm
{
public:
    std::unordered_map<Window, frame_info> frame_list;
    static wm *get();
    void init();
    void exit();
    void run();
    void frame_window(Window window);
    void handle_event(XEvent event);

    int wm_detected(Display *current_display, XErrorEvent *error);
    Display *get_display() { return current_display; };
    Window get_window() { return main_window; };
    XWindowAttributes last_focused_window;
    std::string terminal_emulator;
    int BORDER_WIDTH;

private:
    Display *current_display;
    Window main_window;
};

inline int on_wm_detected(Display *d, XErrorEvent *event)
{
    return wm::get()->wm_detected(d, event);
};

} // namespace idkWM
#endif
