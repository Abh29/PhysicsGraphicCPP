#ifndef FTGRAPHICS_FT_PHYSICALDEVICE_H
#define FTGRAPHICS_FT_PHYSICALDEVICE_H

#include "ft_headers.h"
#include "ft_defines.h"
#include "ft_instance.h"
#include "ft_surface.h"

namespace ft {

	class PhysicalDevice {
	public:
		using pointer = std::shared_ptr<PhysicalDevice>;

		PhysicalDevice(Instance::pointer &instance,
					   Surface::pointer &surface,
					   std::vector<const char *> &deviceExtensions);
		~PhysicalDevice();

		[[nodiscard]] VkPhysicalDevice getVKPhysicalDevice() const;
		[[nodiscard]] VkSampleCountFlagBits getMSAASamples() const;
		[[nodiscard]] QueueFamilyIndices getQueueFamilyIndices() const;
		[[nodiscard]] VkPhysicalDeviceMemoryProperties getPhysicalDeviceMemoryProperties() const;
		[[nodiscard]] VkPhysicalDeviceProperties getPhysicalDeviceProperties() const;
		VkFormatProperties getFormatProperties(VkFormat& format) const;

	private:

		void pickPhysicalDevice();
		bool isDeviceSuitable(VkPhysicalDevice device);
		uint32_t rateDeviceSuitability(VkPhysicalDevice device);
		VkSampleCountFlagBits getMaxUsableSampleCount();
		QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
		bool checkDeviceExtensionSupport(VkPhysicalDevice device);
		SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);



		Instance::pointer 					_ftInstance;
		Surface::pointer					_ftSurface;
		VkPhysicalDevice					_physicalDevice;
		VkSampleCountFlagBits 				_msaaSamples;
		std::vector<const char *>			_deviceExtensions;
		QueueFamilyIndices					_queueFamilyIndices;

	};

}


#endif //FTGRAPHICS_FT_PHYSICALDEVICE_H
