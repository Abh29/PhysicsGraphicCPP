#ifndef FTGRAPHICS_FT_DEVICE_H
#define FTGRAPHICS_FT_DEVICE_H

#include "ft_physicalDevice.h"
#include "ft_instance.h"

namespace ft {

	class Device {
	public:

		using pointer = std::shared_ptr<Device>;

		Device(PhysicalDevice::pointer &physicalDevice,
			   std::vector<const char *> &validationLayers,
			   std::vector<const char *> &deviceExtensions);
		~Device();
		Device(const Device& other) = delete;
		Device operator=(const Device& other) = delete;


		[[nodiscard]] VkDevice  getVKDevice() const;
		[[nodiscard]] VkQueue   getVKGraphicsQueue() const;
		[[nodiscard]] VkQueue   getVKPresentQueue() const;
		uint32_t  findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
		[[nodiscard]] VkSampleCountFlagBits getMSAASamples() const;
		[[nodiscard]] VkPhysicalDeviceProperties getVKPhysicalDeviceProperties() const;
		VkFormatProperties getVKFormatProperties(VkFormat& format) const;
		[[nodiscard]] VkFormat selectSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features) const;
		[[nodiscard]] VkFormat findDepthFormat() const;
		[[nodiscard]] bool hasStencilComponent(VkFormat format) const;
		[[nodiscard]] QueueFamilyIndices getQueueFamilyIndices() const;

	private:
		PhysicalDevice::pointer				_ftPhysicalDevice;
		VkDevice 							_device;
		VkQueue 							_graphicsQueue;
		VkQueue 							_presentQueue;
	};

}


#endif //FTGRAPHICS_FT_DEVICE_H
