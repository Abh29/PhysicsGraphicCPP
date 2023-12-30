#include "../includes/ft_event.h"

std::vector<std::any> ft::Event::getData() const {return _data;}

/******************************MouseEvent**************************************/

ft::CursorEvent::CursorEvent(int button, int action, int mods, double x, double y) {
	_data.push_back(button);
	_data.push_back(action);
	_data.push_back(mods);
	_data.push_back(x);
	_data.push_back(y);
}

ft::Event::EventType ft::CursorEvent::getType() const {return EventType::MOUSE_BUTTON;}

/*******************************KeyBoardEvent**********************************/

ft::KeyboardEvent::KeyboardEvent(int key, int scancode, int action, int mods) {
	_data.push_back(key);
	_data.push_back(scancode);
	_data.push_back(action);
	_data.push_back(mods);
}

ft::Event::EventType ft::KeyboardEvent::getType() const {return Event::EventType::KEYBOARD_EVENT;}

/**********************************EventListener****************************/
void ft::EventListener::addCallbackForEventType(Event::EventType et, std::function<void(Event&)> &&callback) {
	_callbacks.insert(std::make_pair(et, std::move(callback)));
}

void ft::EventListener::fireEvent(Event &ev) const {
	auto f = _callbacks.find(ev.getType());
	if (f == _callbacks.end()) return;
	f->second(ev);
}

/***********************************ScrollEvent******************************/
ft::ScrollEvent::ScrollEvent(double xOffset, double yOffset, double x, double y) {
	_data.push_back(xOffset);
	_data.push_back(yOffset);
	_data.push_back(x);
	_data.push_back(y);
}

ft::Event::EventType ft::ScrollEvent::getType() const {return EventType::MOUSE_SCROLL;}
