#ifndef FTGRAPHICS_FT_DEVICE_H
#define FTGRAPHICS_FT_DEVICE_H

#include "ft_headers.h"
#include "ft_instance.h"
#include "ft_physicalDevice.h"


namespace ft {

	class PhysicalDevice;
	struct QueueFamilyIndices;


	class Device {

	public:

		Device(std::shared_ptr<PhysicalDevice>& physicalDevice,
			   std::vector<const char *> &validationLayers,
			   std::vector<const char *> &deviceExtensions);
		~Device();


		VkDevice  getVKDevice() const;
		VkQueue   getVKGraphicsQueue() const;
		VkQueue   getVKPresentQueue() const;
		uint32_t  findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

	private:
		std::shared_ptr<PhysicalDevice>		_ftPhysicalDevice;
		VkDevice 							_device;
		VkQueue 							_graphicsQueue;
		VkQueue 							_presentQueue;
	};

}


#endif //FTGRAPHICS_FT_DEVICE_H
