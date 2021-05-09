#include "tiling.hpp"
#include "idkWM.hpp"
#include "log.hpp"
namespace idkWM
{
tiling main_tiling;

tiling *tiling::get()
{
    return &main_tiling;
}
void tiling::tile(const XMapRequestEvent event)
{
    int snum = DefaultScreen(wm::get()->get_display());

    int width = DisplayWidth(wm::get()->get_display(), snum);
    int height = DisplayHeight(wm::get()->get_display(), snum);

    const Window frame = wm::get()->frame_list[event.window].frame;

    if (!tiling::master_window)
    {

        XResizeWindow(wm::get()->get_display(), event.window, width, height);
        XResizeWindow(wm::get()->get_display(), frame, width, height);

        // Get window info

        XWindowAttributes attr;
        XGetWindowAttributes(wm::get()->get_display(), event.window, &attr);

        // Move the window to the center
        XMoveWindow(wm::get()->get_display(), frame, (width - attr.width) / 2, (height - attr.height) / 2);
        XMoveWindow(wm::get()->get_display(), event.window, (width - attr.width) / 2, (height - attr.height) / 2);

        master_window = event.window;
    }

    else
    {
        if (window_list.size() == 0)
        {

            // Resize the master window
            XResizeWindow(wm::get()->get_display(), master_window, width / 2, height);
            XResizeWindow(wm::get()->get_display(), wm::get()->frame_list[master_window].frame, width / 2, height);

            // Get attributes of master window
            XWindowAttributes attr;
            XGetWindowAttributes(wm::get()->get_display(), master_window, &attr);

            // Resize the other window
            XResizeWindow(wm::get()->get_display(), event.window, attr.width, height);
            XResizeWindow(wm::get()->get_display(), frame, attr.width, height);

            // Move the other window
            XMoveWindow(wm::get()->get_display(), event.window, attr.width, attr.y);
            XMoveWindow(wm::get()->get_display(), frame, attr.width, attr.y);
            window_list.push_back(event.window);
        }
        else
        {
            window_list.push_back(event.window);

            for (size_t i = 0; i < window_list.size(); i++)
            {
                if (!window_list[i - 1])
                {
                    XWindowAttributes attr;
                    XGetWindowAttributes(wm::get()->get_display(), window_list[0], &attr);

                    XResizeWindow(wm::get()->get_display(), window_list[0], attr.width, attr.height / 2);
                    XResizeWindow(wm::get()->get_display(), wm::get()->frame_list[window_list[0]].frame, attr.width, attr.height / 2);
                }
                else
                {
                    XWindowAttributes attr;
                    XGetWindowAttributes(wm::get()->get_display(), window_list[i], &attr);

                    XResizeWindow(wm::get()->get_display(), window_list[i], attr.width, attr.height / 2);
                    XResizeWindow(wm::get()->get_display(), wm::get()->frame_list[window_list[i]].frame, attr.width, attr.height / 2);

                    // Get window name
                    char *name;
                    XFetchName(wm::get()->get_display(), window_list[i], &name);

                    log("tiling %s with i %d", name, i - 1);

                    XWindowAttributes nattr;
                    XGetWindowAttributes(wm::get()->get_display(), window_list[i - 1], &nattr);

                    XResizeWindow(wm::get()->get_display(), window_list[i], nattr.width, nattr.height * 2);
                    XResizeWindow(wm::get()->get_display(), wm::get()->frame_list[window_list[i + 1]].frame, nattr.width, nattr.height * 2);

                    XMoveWindow(wm::get()->get_display(), window_list[i], nattr.x, nattr.height);
                    XMoveWindow(wm::get()->get_display(), frame, nattr.x, nattr.height - nattr.y);
                }
            }
        }
    }
}
} // namespace idkWM
