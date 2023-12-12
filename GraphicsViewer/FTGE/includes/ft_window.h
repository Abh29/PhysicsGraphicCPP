#ifndef FTGRAPHICS_FT_WINDOW_H
#define FTGRAPHICS_FT_WINDOW_H

#include "ft_headers.h"
#include "ft_instance.h"
#include "ft_event.h"
#include "ft_defines.h"

namespace ft {

	class Instance;

	class Window {

	public:
		using pointer = std::shared_ptr<Window>;

		Window(uint32_t w, uint32_t h, std::string name, GLFWframebuffersizefun resizeCallback = nullptr, EventListener::pointer eventListener = nullptr);
		~Window();
		Window(const Window& other) = delete;
		Window operator=(const Window& other) = delete;

		bool shouldClose();
		void pollEvents();
		void* getRawWindowPointer() const;
		[[nodiscard]] std::vector<const char *> getRequiredExtensions() const;
		VkSurfaceKHR createVKSurface(const Instance& instance);
		std::pair<uint32_t, uint32_t> queryCurrentWidthHeight();
		void waitEvents();
		EventListener::pointer& getEventListener();
		int KEY(KeyboardKeys key) const;
		int MOUSE(MouseKeys key) const;
		int ACTION(KeyActions action) const;

	private:

		void initWindow();
		static void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods);
		void initKeys();

		uint32_t 									_width;
		uint32_t 									_height;
		std::string 								_name;
		GLFWwindow*									_window;
		GLFWframebuffersizefun 						_resizeCallback;
		EventListener::pointer 						_ftEventListener;
		std::unordered_map<KeyboardKeys, int> 		_keys;
		std::unordered_map<MouseKeys, int>			_mouseKeys;
		std::unordered_map<KeyActions, int>			_keyActions;
	};
}

#endif //FTGRAPHICS_FT_WINDOW_H
