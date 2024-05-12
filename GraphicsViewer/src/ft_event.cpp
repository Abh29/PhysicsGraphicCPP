#include "../includes/ft_event.h"
#include <algorithm>
#include <memory>
#include <mutex>
#include <utility>

std::vector<std::any> ft::Event::getData() const { return _data; }

/******************************MouseEvent**************************************/

ft::CursorEvent::CursorEvent(int button, int action, int mods, double x,
                             double y) {
  _data.push_back(button);
  _data.push_back(action);
  _data.push_back(mods);
  _data.push_back(x);
  _data.push_back(y);
}

ft::Event::EventType ft::CursorEvent::getType() const {
  return EventType::MOUSE_BUTTON;
}

/*******************************KeyBoardEvent**********************************/

ft::KeyboardEvent::KeyboardEvent(int key, int scancode, int action, int mods) {
  _data.push_back(key);
  _data.push_back(scancode);
  _data.push_back(action);
  _data.push_back(mods);
}

ft::Event::EventType ft::KeyboardEvent::getType() const {
  return Event::EventType::KEYBOARD_EVENT;
}

/**********************************EventListener****************************/
void ft::EventListener::addCallbackForEventType(
    Event::EventType et, std::function<void(Event &)> &&callback) {
  bool found = false;
  for (auto &p : _callbacks) {
    if (p.first == et) {
      found = true;
      p.second = std::move(callback);
      break;
    }
  }
  if (!found)
    _callbacks.push_back(std::make_pair(et, std::move(callback)));
}

void ft::EventListener::fireInstante(Event &ev) const {
  for (auto &p : _callbacks) {
    if (p.first == ev.getType())
      p.second(ev);
  }
}

void ft::EventListener::pushEvent(Event::uniq_ptr ev) {
  std::scoped_lock<std::mutex> lock(_mutex);
  _eventsQueue.push(std::move(ev));
}

ft::Event::uniq_ptr ft::EventListener::popEvent() {
  std::scoped_lock<std::mutex> lock(_mutex);
  if (!_eventsQueue.empty()) {
    auto out = std::move(_eventsQueue.front());
    _eventsQueue.pop();
    return out;
  }
  return std::make_unique<EmptyEvent>();
}

bool ft::EventListener::isQueueEmpty() const {
  std::scoped_lock<std::mutex> lock(_mutex);
  return _eventsQueue.empty();
}

/***********************************ScrollEvent******************************/
ft::ScrollEvent::ScrollEvent(double xOffset, double yOffset, double x,
                             double y) {
  _data.push_back(xOffset);
  _data.push_back(yOffset);
  _data.push_back(x);
  _data.push_back(y);
}

ft::Event::EventType ft::ScrollEvent::getType() const {
  return EventType::MOUSE_SCROLL;
}

/***********************************ScreenResize*********************************/
ft::ScreenResizeEvent::ScreenResizeEvent(int width, int height) {
  _data.push_back(width);
  _data.push_back(height);
}

ft::Event::EventType ft::ScreenResizeEvent::getType() const {
  return EventType ::SCREEN_RESIZE_EVENT;
}

/***********************************CursorDrag*********************************/
ft::CursorDragEvent::CursorDragEvent(int button, double x, double y,
                                     double oldx, double oldy) {
  _data.push_back(button);
  _data.push_back(x);
  _data.push_back(y);
  _data.push_back(oldx);
  _data.push_back(oldy);
}

ft::Event::EventType ft::CursorDragEvent::getType() const {
  return EventType::MOUSE_BUTTON_DRAG;
}

/***********************************CursorDragRelease*********************************/
ft::CursorDragReleaseEvent::CursorDragReleaseEvent(int button, double x,
                                                   double y) {
  _data.push_back(button);
  _data.push_back(x);
  _data.push_back(y);
}

ft::Event::EventType ft::CursorDragReleaseEvent::getType() const {
  return EventType::MOUSE_BUTTON_DRAG_RELEASE;
}
