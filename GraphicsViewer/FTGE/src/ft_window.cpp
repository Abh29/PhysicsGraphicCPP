#include "../includes/ft_window.h"


ft::Window::Window(uint32_t w, uint32_t h, std::string name, GLFWframebuffersizefun resizeCallback, GLFWkeyfun keyCallback) :
_width(w), _height(h), _name(name), _resizeCallback(resizeCallback), _keyCallback(keyCallback) {
	initWindow();
}

ft::Window::~Window() {
	glfwDestroyWindow(_window);
	glfwTerminate();
}

void ft::Window::initWindow() {
	if (glfwInit() != GLFW_TRUE)
		throw std::runtime_error("GLFW could not initialize a window!");

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE); // remove this later

	_window = glfwCreateWindow(_width, _height, _name.c_str(), nullptr, nullptr);
	glfwSetWindowUserPointer(_window, this);
	glfwSetFramebufferSizeCallback(_window, _resizeCallback);
	glfwSetKeyCallback(_window, _keyCallback);
}

bool ft::Window::shouldClose() {
	return glfwWindowShouldClose(_window);
}

void ft::Window::pollEvents() {
	glfwPollEvents();
}

void ft::Window::setResized(bool v) {_resized = v;}
