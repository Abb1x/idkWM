#ifndef EVENTS_H
#define EVENTS_H
#include "idkWM.hpp"
#include "log.hpp"
namespace idkWM
{
namespace events
{
struct position
{

    int x, y;
};
bool create(const XConfigureRequestEvent event);
bool key_press(const XKeyEvent &event);
bool key_release(const XKeyEvent &event);
bool button_event(const XButtonEvent &event);
bool button_release(const XButtonEvent &event);
bool motion(const XMotionEvent &event);
bool enter(XEnterWindowEvent &event);
bool map(const XMapRequestEvent event);
bool unmap(const XUnmapEvent &event);
} // namespace events
} // namespace idkWM

#endif
