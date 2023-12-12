#include "../include.h"

std::vector<std::any> ft::Event::getData() const {return _data;}

/******************************MouseEvent**************************************/

ft::Event::EventType ft::MouseEvent::getType() const {return EventType::MOUSE_EVENT;}

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
