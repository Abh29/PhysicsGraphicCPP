#include "../includes/ft_callbacks.h"
#include "../includes/ft_window.h"

void ft::Callback::keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods) {
	if (action == GLFW_PRESS) {
		switch (key) {
			case GLFW_KEY_ESCAPE:
				glfwSetWindowShouldClose(window, true);
				break;
			default :
				std::cout << "key pressed " << key << ", " << scancode << ", " << mods << std::endl;
		}
	}
}

void ft::Callback::resizeCallback(GLFWwindow *window, int w, int h) {
	auto *ftw = reinterpret_cast<ft::Window*>(glfwGetWindowUserPointer(window));
	ftw->setResized(true);
	(void) w;
	(void) h;
}

