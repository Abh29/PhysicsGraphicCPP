#ifndef FTGRAPHICS_FT_SURFACE_H
#define FTGRAPHICS_FT_SURFACE_H

#include "ft_headers.h"
#include "ft_window.h"

namespace ft {

	class Instance;
	class Window;

	class Surface {
	public:

		using pointer = std::shared_ptr<Surface>;

		Surface(std::shared_ptr<Instance> instance,
				std::shared_ptr<Window> window);
		~Surface();

		VkSurfaceKHR  getVKSurface() const;

	private:
		VkSurfaceKHR 						_surface = VK_NULL_HANDLE;
		std::shared_ptr<Instance>			_ftInstance;
		std::shared_ptr<Window>				_ftWindow;
	};
}

#endif //FTGRAPHICS_FT_SURFACE_H
