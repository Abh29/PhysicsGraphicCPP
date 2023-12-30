#include "../includes/ft_surface.h"

ft::Surface::Surface(std::shared_ptr<Instance> instance,
					 std::shared_ptr<Window> window) :
					 _ftInstance(instance),
					 _ftWindow(window)
					{
						_surface = window->createVKSurface(*_ftInstance);
					}

ft::Surface::~Surface() {
	vkDestroySurfaceKHR(_ftInstance->getVKInstance(), _surface, nullptr);
}

VkSurfaceKHR ft::Surface::getVKSurface() const {return _surface;}