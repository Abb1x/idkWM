#include "idkWM.hpp"
#include "../lib/json.h"
#include "events.hpp"
#include "log.hpp"
#include <fstream>
#include <iostream>
#include <mutex>
namespace idkWM
{
std::mutex main_mutex;
const char *event_string_list[] = {
    "null",
    "null",
    "key press",
    "key release",
    "button press",
    "button release",
    "enter notify",
    "leave notify",
    "focus in",
    "focus out",
    "keymap notify",
    "expose",
    "graphics expose",
    "no expose",
    "visibility notify",
    "create notify",
    "destroy notify",
    "unmap notify",
    "map notify",
    "map request",
    "reparent notify",
    "configure notify",
    "configure request",
    "null",
    "null",
    "null",
    "null",
    "null",
    "null",
    "null",
    "null",
    "null",
    "null",
    "null",
    "null",
    "null",
    "null",
    "null",
    "null",
    "null",
};
wm main_wm;
unsigned long BORDER_COLOR = 0;
int border_width = 0;
wm *wm::get()
{
    return &main_wm;
}

void wm::init()
{

    log("Initializing idkWM...");
    current_display = XOpenDisplay(0);

    if (!current_display)
    {
        log("ERROR! Can't open display");
        std::exit(-1);
    }

    else
    {
        log("Found display %s", XDisplayName(nullptr));
    }

    main_window = DefaultRootWindow(current_display);

    log("idkWM initialized!");

    std::ifstream config_file((std::string)getenv("HOME") + "/.config/idkWM/config.json");
    std::string line;
    std::string final;

    if (config_file.is_open())
    {
        while (getline(config_file, line))
        {
            final += line;
        }
        config_file.close();
    }
    json::jobject result = json::jobject::parse(final);

    std::string border_color_str = result.get("border_color");
    std::string border_width_str = result.get("border_width");
    std::string border_color_converted = JSON_GETLONG(border_color_str);
    std::string border_width_converted = JSON_GETLONG(border_width_str);
    
    BORDER_COLOR = std::stoul(border_color_converted.c_str(), nullptr, 0);
    border_width = std::stoi(border_width_converted.c_str(),nullptr,0);
}

void wm::run()
{

    log("Running idkWM...");

    // Set error handler
    XSetErrorHandler(on_wm_detected);
    // select the events that we want reported to our window manager
    XSelectInput(current_display, main_window, SubstructureRedirectMask | SubstructureNotifyMask);

    // Main loop
    for (;;)
    {
        main_mutex.lock();
        // Get next event
        XEvent current_event;
        XNextEvent(current_display, &current_event);

        handle_event(current_event);
        log("Receiving event with type: %s", event_string_list[current_event.type]);

        main_mutex.unlock();
        usleep(10);
    }
}
void wm::exit()
{
    log("Exiting...");
    XCloseDisplay(current_display);
    std::exit(1);
}

void wm::frame_window(Window window)
{
    if (frame_list.count(window))
    {
        return;
    }
    const unsigned long BG_COLOR = 0x0000ff;

    XWindowAttributes window_attributes;
    XGetWindowAttributes(current_display, window, &window_attributes);

    Window on_top = XCreateSimpleWindow(current_display, main_window, window_attributes.x, window_attributes.y, window_attributes.width, window_attributes.height, border_width, BORDER_COLOR, BG_COLOR);

    // Select events
    XSelectInput(current_display, on_top, SubstructureRedirectMask | SubstructureNotifyMask);
    // Add client to save set
    XAddToSaveSet(current_display, window);
    // Reparent window
    XReparentWindow(current_display, window, on_top, 0, 0);
    // Map frame
    XMapWindow(current_display, on_top);

    // Set attributes
    frame_list[window].frame = on_top;
    frame_list[window].movable = true;
    frame_list[window].resizable = true;
    frame_list[window].display = current_display;
    frame_list[window].created = true;

    frame_list[window].x = window_attributes.x;
    frame_list[window].y = window_attributes.y;
    frame_list[window].width = window_attributes.width;
    frame_list[window].height = window_attributes.height;

    XGrabButton(
        current_display,
        Button1,
        AnyModifier,
        window,
        false,
        ButtonPressMask | ButtonReleaseMask | ButtonMotionMask,
        GrabModeAsync,
        GrabModeAsync,
        None,
        None);
    XGrabButton(
        current_display,
        Button3,
        AnyModifier,
        window,
        false,
        ButtonPressMask | ButtonReleaseMask | ButtonMotionMask,
        GrabModeAsync,
        GrabModeAsync,
        None,
        None);

    XGrabKey(
        current_display,
        XKeysymToKeycode(current_display, XK_r),
        AnyModifier,
        main_window,
        false,
        GrabModeAsync,
        GrabModeAsync);
    XGrabKey(
        current_display,
        XKeysymToKeycode(current_display, XK_c),
        AnyModifier,
        window,
        false,
        GrabModeAsync,
        GrabModeAsync);
}

void wm::handle_event(XEvent event)
{
    switch (event.type)
    {
    case ConfigureRequest:
        events::create(event.xconfigurerequest);
        break;
    case MapRequest:
        events::map(event.xmaprequest);
        break;
    case UnmapNotify:
        events::unmap(event.xunmap);
        break;
    case ButtonPress:
        events::button_event(event.xbutton);
        break;
    case KeyPress:
        events::key_press(event.xkey);
        break;
    case ButtonRelease:
        break;
    case MotionNotify:
        while (XCheckTypedWindowEvent(current_display, event.xmotion.window, MotionNotify, &event))
        {
            // skip pending operation
        }
        events::motion(event.xmotion);
        break;
    default:
        break;
    }
}

int wm::wm_detected(Display *current_display, XErrorEvent *error)
{
    if (error->error_code == BadAccess)
    {
        log("ERROR! Another window manager is already running");
        exit();
        return 1;
    }
    return 0;
}
} // namespace idkWM
