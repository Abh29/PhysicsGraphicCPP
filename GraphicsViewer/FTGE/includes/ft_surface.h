#ifndef FTGRAPHICS_FT_SURFACE_H
#define FTGRAPHICS_FT_SURFACE_H

#include "ft_headers.h"

namespace ft {

	class Instance;
	class Window;

	class Surface {
	public:
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
