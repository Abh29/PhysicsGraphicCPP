#ifndef FTGRAPHICS_FT_WINDOW_H
#define FTGRAPHICS_FT_WINDOW_H

#include "ft_headers.h"

namespace ft {

	class Window {

	public:

		Window(uint32_t w, uint32_t h, std::string name, GLFWframebuffersizefun resizeCallback = nullptr, GLFWkeyfun keyCallback = nullptr);
		~Window();
		Window(const Window& other) = delete;
		Window operator=(const Window& other) = delete;

		bool shouldClose();
		void pollEvents();
		// this could fire an event on resize 
		void setResized(bool v);


	private:

		void initWindow();


		uint32_t 					_width;
		uint32_t 					_height;
		std::string 				_name;
		GLFWwindow*					_window;
		GLFWframebuffersizefun 		_resizeCallback;
		GLFWkeyfun 					_keyCallback;
		bool 						_resized = false;
	};
}

#endif //FTGRAPHICS_FT_WINDOW_H
