#ifndef FTGRAPHICS_FT_APP_H
#define FTGRAPHICS_FT_APP_H


#include "ft_headers.h"
#include "ft_window.h"
#include "ft_instance.h"
#include "ft_surface.h"
#include "ft_physicalDevice.h"
#include "ft_device.h"

namespace ft {

	class Application {
	public:
		static constexpr uint32_t W_WIDTH = 800;
		static constexpr uint32_t W_HEIGHT = 600;

		Application();
		~Application();

		void run();

	private:
		void initApplication();


		std::shared_ptr<Window>				_ftWindow;
		std::shared_ptr<Instance>			_ftInstance;
		std::shared_ptr<Surface>			_ftSurface;
		std::shared_ptr<PhysicalDevice>		_ftPhysicalDevice;
		std::shared_ptr<Device>				_ftDevice;
		std::shared_ptr<SwapChain>			_ftSwapChain;
		std::vector<const char*> 			_validationLayers;
		std::vector<const char*> 			_deviceExtensions;
	};

}

#endif //FTGRAPHICS_FT_APP_H
