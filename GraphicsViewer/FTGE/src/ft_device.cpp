#include "../include.h"
#include "../includes/ft_device.h"


ft::Device::Device(std::shared_ptr<PhysicalDevice>& physicalDevice,
				   std::vector<const char *> &validationLayers,
				   std::vector<const char *> &deviceExtensions):
		_ftPhysicalDevice(physicalDevice)
		{
			ft::QueueFamilyIndices indices = _ftPhysicalDevice->getQueueFamilyIndices();
			std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
			std::set<uint32_t> uniqueQueueFamilies = {indices.graphicsFamily.value(), indices.presentFamily.value()};

			float queuePriority = 1.0f;
			for (uint32_t queueFamily : uniqueQueueFamilies) {
				VkDeviceQueueCreateInfo queueCreateInfo{};
				queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
				queueCreateInfo.queueFamilyIndex = queueFamily;
				queueCreateInfo.queueCount = 1;
				queueCreateInfo.pQueuePriorities = &queuePriority;

				queueCreateInfos.push_back(queueCreateInfo);
			}

			VkPhysicalDeviceFeatures deviceFeatures{};
			deviceFeatures.samplerAnisotropy = VK_TRUE;
			//		deviceFeatures.sampleRateShading = VK_TRUE; // this is for sample shading

			VkDeviceCreateInfo deviceCreateInfo{};
			deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
			deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
			deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
			deviceCreateInfo.pEnabledFeatures = &deviceFeatures;
			deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
			deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions.data();
			if (enableValidationLayers) {
				deviceCreateInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
				deviceCreateInfo.ppEnabledLayerNames = validationLayers.data();
			} else {
				deviceCreateInfo.enabledLayerCount = 0;
			}

			if (vkCreateDevice(_ftPhysicalDevice->getVKPhysicalDevice(), &deviceCreateInfo, nullptr, &_device) != VK_SUCCESS)
				throw std::runtime_error("failed to create a logical device!");

			vkGetDeviceQueue(_device, indices.graphicsFamily.value(), 0, &_graphicsQueue);
			vkGetDeviceQueue(_device, indices.presentFamily.value(), 0, &_presentQueue);

		}


ft::Device::~Device() {
	vkDestroyDevice(_device, nullptr);
}


VkDevice ft::Device::getVKDevice() const {return _device;}
VkQueue ft::Device::getVKGraphicsQueue() const {return _graphicsQueue;}
VkQueue ft::Device::getVKPresentQueue() const {return _presentQueue;}

uint32_t ft::Device::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
	VkPhysicalDeviceMemoryProperties deviceMemoryProperties = _ftPhysicalDevice->getPhysicalDeviceMemoryProperties();

	for (uint32_t i = 0; i < deviceMemoryProperties.memoryTypeCount; ++i) {
		if (typeFilter & (1 << i) &&
			(deviceMemoryProperties.memoryTypes[i].propertyFlags & properties) == properties) {
			return i;
		}
	}

	throw std::runtime_error("failed to find suitable memory type!");
}

