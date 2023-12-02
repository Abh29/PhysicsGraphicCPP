#ifndef FTGRAPHICS_FT_PHYSICALDEVICE_H
#define FTGRAPHICS_FT_PHYSICALDEVICE_H

#include "ft_headers.h"
#include "ft_instance.h"
#include "ft_swapChain.h"

namespace ft {

	class Instance;
	class Surface;

	struct QueueFamilyIndices {
		std::optional<uint32_t> graphicsFamily;
		std::optional<uint32_t> presentFamily;

		[[nodiscard]] bool isComplete() const;
	};

	struct SwapChainSupportDetails {
		VkSurfaceCapabilitiesKHR capabilities;
		std::vector<VkSurfaceFormatKHR> formats;
		std::vector<VkPresentModeKHR> presentModes;
	};


	class PhysicalDevice {
	public:

		PhysicalDevice(std::shared_ptr<Instance> &instance,
					   std::shared_ptr<Surface> &surface,
					   std::vector<const char *> &deviceExtensions);
		~PhysicalDevice();

		VkPhysicalDevice getVKPhysicalDevice() const;
		VkSampleCountFlagBits getMSAASamples() const;
		QueueFamilyIndices getQueueFamilyIndices() const;


	private:

		void pickPhysicalDevice();
		bool isDeviceSuitable(VkPhysicalDevice device);
		uint32_t rateDeviceSuitability(VkPhysicalDevice device);
		VkSampleCountFlagBits getMaxUsableSampleCount();
		QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
		bool checkDeviceExtensionSupport(VkPhysicalDevice device);
		SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);



		std::shared_ptr<Instance>			_ftInstance;
		std::shared_ptr<Surface>			_ftSurface;
		VkPhysicalDevice					_physicalDevice;
		VkSampleCountFlagBits 				_msaaSamples;
		std::vector<const char *>			_deviceExtensions;
		QueueFamilyIndices					_queueFamilyIndices;

	};

}


#endif //FTGRAPHICS_FT_PHYSICALDEVICE_H
