#include "events.hpp"
#include "tiling.hpp"
#include <X11/Xatom.h>
#include <X11/Xlib.h>
namespace idkWM
{
position last_mouse_click;

bool events::create(const XConfigureRequestEvent event)
{
    log("Creating new window %i", event.window);

    XWindowChanges final_window;

    // Copy fields from event to final_window
    final_window.x = event.x;
    final_window.y = event.y;
    final_window.width = event.width;
    final_window.height = event.height;
    final_window.border_width = event.border_width;
    final_window.sibling = event.above;
    final_window.stack_mode = event.detail;

    XConfigureWindow(wm::get()->get_display(), event.window, event.value_mask, &final_window);

    log("Resized %i to %ix%i", event.window, event.width, event.height);

    return true;
}

bool events::key_press(const XKeyEvent &event)
{
    log("Key pressed: %x", event.keycode);

    // super + r = opens dmenu
    if ((event.state & (ShiftMask | ControlMask | Mod1Mask | Mod4Mask)) == (Mod4Mask) && event.keycode == XKeysymToKeycode(wm::get()->get_display(), XK_r))
    {
        wm::get()->spawn("dmenu_run");
    }
    else
    {
    }
    // Super + c = closes focused window
    if ((event.state & (ShiftMask | ControlMask | Mod1Mask | Mod4Mask)) == (ShiftMask | Mod4Mask) && event.keycode == XKeysymToKeycode(wm::get()->get_display(), XK_c))
    {
        XGrabServer(wm::get()->get_display());
        XSetCloseDownMode(wm::get()->get_display(), DestroyAll);
        XKillClient(wm::get()->get_display(), event.window);
        XSync(wm::get()->get_display(), False);
        XUngrabServer(wm::get()->get_display());
    }

    // Ctrl + alt + t = opens terminal
    if ((event.state & (ShiftMask | ControlMask | Mod1Mask | Mod4Mask)) == (ControlMask | Mod1Mask) && event.keycode == XKeysymToKeycode(wm::get()->get_display(), XK_t))
    {
        wm::get()->spawn(wm::get()->terminal_emulator);
    }
    return true;
}
Window focused_win;
bool events::button_event(const XButtonEvent &event)
{
    const Window target = wm::get()->frame_list[event.window].frame;
    Atom focused = XInternAtom(wm::get()->get_display(), "_NET_ACTIVE_WINDOW", False);

    last_mouse_click = {event.x_root, event.y_root};
    XRaiseWindow(wm::get()->get_display(), target);

    if (focused_win != event.window || (event.button != Button4 && event.button != Button5))
    {
        XSetInputFocus(wm::get()->get_display(), wm::get()->get_window(), RevertToPointerRoot, CurrentTime);
        XDeleteProperty(wm::get()->get_display(), wm::get()->get_window(), focused);

        // Focus
        XSetInputFocus(wm::get()->get_display(), event.window, RevertToPointerRoot, CurrentTime);
        XChangeProperty(wm::get()->get_display(), wm::get()->get_window(), focused, XA_WINDOW, 32, PropModeReplace, (unsigned char *)&(event.window), 1);
    }
    XGetWindowAttributes(wm::get()->get_display(), target, &wm::get()->last_focused_window);
    focused_win = event.window;

    return true;
}
bool events::button_release(const XButtonEvent &event) { return true; };
bool events::motion(const XMotionEvent &event)
{
    position final = {event.x_root, event.y_root};

    final.x -= last_mouse_click.x;
    final.y -= last_mouse_click.y;

    const Window frame = wm::get()->frame_list[event.window].frame;
    if ((event.state & (ShiftMask | ControlMask | Mod1Mask | Mod4Mask)) == (Mod4Mask) && event.state & Button1Mask && wm::get()->frame_list[event.window].movable)
    {

        XMoveWindow(wm::get()->get_display(), frame, final.x, final.y);
    }

    else if ((event.state & (ShiftMask | ControlMask | Mod1Mask | Mod4Mask)) == (Mod4Mask) && event.state & Button3Mask && wm::get()->frame_list[event.window].resizable)
    {

        position resized = {
            std::max(final.x + wm::get()->last_focused_window.width, 1),
            std::max(final.y + wm::get()->last_focused_window.height, 1)};

        position dest_resize = {resized.x, resized.y};

        XResizeWindow(wm::get()->get_display(), event.window, dest_resize.x, dest_resize.y);
        XResizeWindow(wm::get()->get_display(), frame, dest_resize.x, dest_resize.y);
    }

    return true;
}

bool events::map(const XMapRequestEvent event)
{


    if (tiling::get()->enabled)
    {
        wm::get()->frame_list[event.window].movable = false;
        wm::get()->frame_list[event.window].resizable = false;
        tiling::get()->tile(event);
    }
    
    wm::get()->frame_window(event.window);


    XMapWindow(wm::get()->get_display(), event.window);

    return true;
}
bool events::unmap(const XUnmapEvent &event)
{
    if (!wm::get()->frame_list.count(event.window))
    {
        return true;
    }

    Window frame = wm::get()->frame_list[event.window].frame;

    XUnmapWindow(wm::get()->get_display(), frame);
    XReparentWindow(wm::get()->get_display(), event.window, wm::get()->get_window(), 0, 0);
    XRemoveFromSaveSet(wm::get()->get_display(), event.window);
    XDestroyWindow(wm::get()->get_display(), frame);

    log("Unframed %i", event.window);

    return true;
}
} // namespace idkWM
