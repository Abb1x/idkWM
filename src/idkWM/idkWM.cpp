#include "idkWM.hpp"
#include "config_parser.hpp"
#include "events.hpp"
#include "log.hpp"
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
int BORDER_WIDTH = 0;
std::vector<std::string> border_exclude;

wm *wm::get()
{
    return &main_wm;
}

void wm::init()
{

    log("Initializing idkWM...");

    current_display = XOpenDisplay(0);
    XCreateFontCursor(current_display, XC_arrow);

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

    terminal_emulator = ConfigParser::get()->get_string("terminal");

    BORDER_COLOR = std::stoul(ConfigParser::get()->get_string("border_color").c_str(), nullptr, 0);

    BORDER_WIDTH = std::stoi(ConfigParser::get()->get_string("border_width").c_str(), nullptr, 0);

    border_exclude = ConfigParser::get()->get_json_array("border_exclude");

    // Set keybindings
    XGrabKey(
        current_display,
        XKeysymToKeycode(current_display, XK_r),
        Mod4Mask,
        main_window,
        false,
        GrabModeAsync,
        GrabModeAsync);

    XGrabKey(
        current_display,
        XKeysymToKeycode(current_display, XK_t),
        ControlMask | Mod1Mask,
        main_window,
        false,
        GrabModeAsync,
        GrabModeAsync);

    // set Atoms
    Atom utf8_string = XInternAtom(current_display, "UTF8_STRING", false);

    Atom netwmcheck = XInternAtom(current_display, "_NET_SUPPORTING_WM_CHECK", false);
    Atom netwmname = XInternAtom(current_display, "_NET_WM_NAME", false);

    Window wm_check_win = XCreateSimpleWindow(current_display, main_window, 0, 0, 1, 1, 0, 0, 0);

    // set name
    XChangeProperty(current_display, wm_check_win, netwmcheck, XA_WINDOW, 32,
                    PropModeReplace, (unsigned char *)&wm_check_win, 1);

    XChangeProperty(current_display, wm_check_win, netwmname, utf8_string, 8,
                    PropModeReplace, (unsigned char *)"idkWM", 5);

    XChangeProperty(current_display, main_window, netwmcheck, XA_WINDOW, 32,
                    PropModeReplace, (unsigned char *)&wm_check_win, 1);
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
	
        main_mutex.unlock();
        usleep(10);
    }
}
char *convert_vector_to_char_array(const std::string &s)
{
    char *pc = new char[s.size() + 1];
    std::strcpy(pc, s.c_str());
    return pc;
}

void wm::spawn(std::string command)
{
    std::vector<std::string> command_array = ConfigParser::get()->split(command, " ");
    
    const char *command_char[command_array.size() + 1];
    
    for (size_t i = 0; i < command_array.size(); i++)
    {
        command_char[i] = command_array[i].c_str();
    }
    command_char[command_array.size()] = NULL;

    if (fork() == 0)
    {
        if (current_display)
            close(ConnectionNumber(current_display));
        setsid();
        execvp(command_char[0],(char **)command_char);
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

    // Get window name
    char *name;
    XFetchName(current_display, window, &name);
    if (!name)
        name = (char *)"Unknown app";

    Window on_top;
    if (std::find(border_exclude.begin(), border_exclude.end(), (std::string)name) == border_exclude.end())
    {
        on_top = XCreateSimpleWindow(current_display, main_window, window_attributes.x, window_attributes.y, window_attributes.width, window_attributes.height, BORDER_WIDTH, BORDER_COLOR, BG_COLOR);
    }
    if (std::find(border_exclude.begin(), border_exclude.end(), (std::string)name) != border_exclude.end())
    {
        on_top = XCreateSimpleWindow(current_display, main_window, window_attributes.x, window_attributes.y, window_attributes.width, window_attributes.height, 0, BORDER_COLOR, BG_COLOR);
    }

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
        Mod4Mask,
        window,
        false,
        ButtonPressMask | ButtonReleaseMask | PointerMotionMask,
        GrabModeAsync,
        GrabModeAsync,
        None,
        None);

    XGrabButton(
        current_display,
        Button3,
        Mod4Mask,
        window,
        false,
        ButtonPressMask | ButtonReleaseMask | PointerMotionMask,
        GrabModeAsync,
        GrabModeAsync,
        None,
        None);

    XGrabKey(
        current_display,
        XKeysymToKeycode(current_display, XK_c),
        ShiftMask | Mod4Mask,
        window,
        false,
        GrabModeAsync,
        GrabModeAsync);
    log("Framed window: %s", name);
}

void wm::handle_event(XEvent event)
{
    switch (event.type)
    {
    case KeyPress:
        events::key_press(event.xkey);
        break;
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
    case ButtonRelease:
        events::button_release(event.xbutton);
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
