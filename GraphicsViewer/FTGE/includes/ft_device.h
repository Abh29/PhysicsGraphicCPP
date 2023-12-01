#ifndef FTGRAPHICS_FT_DEVICE_H
#define FTGRAPHICS_FT_DEVICE_H

#include "ft_headers.h"
#include "ft_physicalDevice.h"
#include "ft_instance.h"

namespace ft {

	class PysicalDevice;

	class Device {

	public:
		Device(std::shared_ptr<PhysicalDevice>& physicalDevice,
			   std::vector<const char *> &validationLayers,
			   std::vector<const char *> &deviceExtensions);
		~Device();


		VkDevice  getVKDevice() const;
		VkQueue   getVKGraphicsQueue() const;
		VkQueue   getVKPresentQueue() const;


	private:
		std::shared_ptr<PhysicalDevice>		_ftPhysicalDevice;
		VkDevice 							_device;
		VkQueue 							_graphicsQueue;
		VkQueue 							_presentQueue;
	};

}


#endif //FTGRAPHICS_FT_DEVICE_H
