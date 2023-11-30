#include "../includes/ft_app.h"
#include "../includes/ft_callbacks.h"

ft::Application::Application() :
_ftWindow{W_WIDTH, W_HEIGHT, "applicationWindow", nullptr, ft::Callback::keyCallback}
{
	initApplication();
}

ft::Application::~Application() {}

void ft::Application::run() {

	while(!_ftWindow.shouldClose()) {
		_ftWindow.pollEvents();
	}
}

void ft::Application::initApplication() {}
