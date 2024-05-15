#ifndef FTGRAPHICS_FT_EVENT_H
#define FTGRAPHICS_FT_EVENT_H

#include "ft_defines.h"
#include "ft_headers.h"
#include <functional>
#include <mutex>
#include <queue>

namespace ft {

class Event {
public:
  using uniq_ptr = std::unique_ptr<Event>;

  using EventType = enum class et {
    // empty
    EMPTY,

    MOUSE_BUTTON,
    MOUSE_BUTTON_DRAG,
    MOUSE_BUTTON_DRAG_RELEASE,
    KEYBOARD_EVENT,
    MOUSE_SCROLL,
    SCREEN_RESIZE_EVENT,

    // gui events
    Menue_File_NEW,
    Menue_File_OPEN,
    Menue_File_SAVE,
    Menue_File_RELOAD,
    Menue_File_SAVEAS,
    Menue_File_QUIT,

    Menue_Insert_MODEL,
    Menue_Insert_SKYBOX,
    Menue_Insert_UBOX,
    Menue_Insert_USPHERE,
    Menue_Insert_Camera,

    Menue_Edit_UNSELECTALL,
    Menue_Edit_UNHIDEALL,
    Menue_Edit_DEFAULTTOPO,
    Menue_Edit_LINETOPO,
    Menue_Edit_POINTTOP,
    Menue_Edit_RECALCNORM,
    Menue_Edit_SHOWNORM,
    Menue_Edit_NEXTCAMERA,
    Menue_Edit_REMOVECAMERA,
  };

  virtual ~Event() = default;
  virtual std::vector<std::any> getData() const;
  [[nodiscard]] virtual EventType getType() const = 0;

protected:
  std::vector<std::any> _data;
};

class EmptyEvent : public Event {
public:
  EmptyEvent() = default;
  ~EmptyEvent() override = default;
  [[nodiscard]] EventType getType() const override {
    return ft::Event::EventType::EMPTY;
  }
};

class StandardEvent : public Event {
public:
  StandardEvent(ft::Event::EventType ev) : _ev(ev) {}
  ~StandardEvent() override = default;

  [[nodiscard]] EventType getType() const override { return _ev; }

private:
  Event::EventType _ev;
};

class CursorEvent : public Event {
public:
  CursorEvent(int button, int action, int mods, double x, double y);
  ~CursorEvent() override = default;
  [[nodiscard]] EventType getType() const override;
};

class CursorDragEvent : public Event {
public:
  CursorDragEvent(int button, double x, double y, double xold, double yold);
  ~CursorDragEvent() override = default;
  [[nodiscard]] EventType getType() const override;
};

class CursorDragReleaseEvent : public Event {
public:
  CursorDragReleaseEvent(int button, double x, double y);
  ~CursorDragReleaseEvent() override = default;
  [[nodiscard]] EventType getType() const override;
};

class KeyboardEvent : public Event {
public:
  KeyboardEvent(int key, int scancode, int action, int mods);
  ~KeyboardEvent() override = default;
  [[nodiscard]] EventType getType() const override;
};

class ScrollEvent : public Event {
public:
  ScrollEvent(double xOffset, double yOffset, double x, double y);
  ~ScrollEvent() override = default;

  [[nodiscard]] EventType getType() const override;
};

class ScreenResizeEvent : public Event {
public:
  ScreenResizeEvent(int width, int height);
  ~ScreenResizeEvent() override = default;

  [[nodiscard]] EventType getType() const override;
};

class EventListener {
public:
  using pointer = std::shared_ptr<EventListener>;
  friend class Application;
  EventListener() = default;
  ~EventListener() = default;

  void addCallbackForEventType(Event::EventType et,
                               std::function<void(Event &)> &&callback);

  void fireInstante(Event &ev) const;
  void pushEvent(Event::uniq_ptr ev);
  Event::uniq_ptr popEvent();
  bool isQueueEmpty() const;

private:
  std::vector<std::pair<Event::EventType, std::function<void(Event &)>>>
      _callbacks;

  std::queue<Event::uniq_ptr> _eventsQueue;
  mutable std::mutex _mutex;
};

} // namespace ft

#endif // FTGRAPHICS_FT_EVENT_H
