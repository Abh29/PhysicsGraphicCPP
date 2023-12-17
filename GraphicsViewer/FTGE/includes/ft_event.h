#ifndef FTGRAPHICS_FT_EVENT_H
#define FTGRAPHICS_FT_EVENT_H

#include "ft_headers.h"
#include "ft_defines.h"

namespace ft {

	class Event {
	public:
		using EventType = enum class et {
			MOUSE_BUTTON,
			KEYBOARD_EVENT,
			MOUSE_SCROLL,
		};
		virtual ~Event() = default;
		virtual std::vector<std::any> getData() const;
		[[nodiscard]] virtual EventType getType() const = 0;

	protected:
		std::vector<std::any>			_data;
	};

	class CursorEvent : public Event {
	public:
		CursorEvent(int button, int action, int mods, double x, double y);
		~CursorEvent() override = default;
		[[nodiscard]] EventType getType() const override;
	};

	class KeyboardEvent : public Event {
	public:
		KeyboardEvent(int key, int scancode, int action, int mods);
		~KeyboardEvent() override = default;
		[[nodiscard]] EventType getType() const override;
	};

	class ScrollEvent: public Event {
	public:
		ScrollEvent(double xOffset, double yOffset, double x, double y);
		~ScrollEvent() override = default;

		[[nodiscard]] EventType getType() const override;
	};

	class EventListener {
	public:
		using pointer = std::shared_ptr<EventListener>;
		friend class Application;
		EventListener() = default;
		~EventListener() = default;

		void addCallbackForEventType(Event::EventType et, std::function<void(Event&)> &&callback);
		void fireEvent(Event &ev) const;
	private:
		std::map<Event::EventType, std::function<void(Event&)>> 	_callbacks;
	};

}

#endif //FTGRAPHICS_FT_EVENT_H
