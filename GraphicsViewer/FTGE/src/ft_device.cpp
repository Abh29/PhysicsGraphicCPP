#include "../includes/ft_device.h"
#include <vulkan/vulkan_core.h>

ft::Device::Device(const std::shared_ptr<PhysicalDevice> &physicalDevice,
                   const std::vector<const char *> &validationLayers,
                   const std::vector<const char *> &deviceExtensions)
    : _ftPhysicalDevice(physicalDevice) {

  ft::QueueFamilyIndices indices = _ftPhysicalDevice->getQueueFamilyIndices();
  std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
  std::set<uint32_t> uniqueQueueFamilies = {indices.graphicsFamily.value(),
                                            indices.presentFamily.value()};

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
  deviceFeatures.geometryShader = VK_TRUE;

  VkPhysicalDeviceExtendedDynamicStateFeaturesEXT ext{};
  ext.sType =
      VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_FEATURES_EXT;
  ext.extendedDynamicState = VK_TRUE;

  VkDeviceCreateInfo deviceCreateInfo{};
  deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
  deviceCreateInfo.pNext = &ext;
  deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
  deviceCreateInfo.queueCreateInfoCount =
      static_cast<uint32_t>(queueCreateInfos.size());
  deviceCreateInfo.pEnabledFeatures = &deviceFeatures;
  deviceCreateInfo.enabledExtensionCount =
      static_cast<uint32_t>(deviceExtensions.size());
  deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions.data();
  if (enableValidationLayers) {
    deviceCreateInfo.enabledLayerCount =
        static_cast<uint32_t>(validationLayers.size());
    deviceCreateInfo.ppEnabledLayerNames = validationLayers.data();
  } else {
    deviceCreateInfo.enabledLayerCount = 0;
  }

  if (vkCreateDevice(_ftPhysicalDevice->getVKPhysicalDevice(),
                     &deviceCreateInfo, nullptr, &_device) != VK_SUCCESS)
    throw std::runtime_error("failed to create a logical device!");

  vkGetDeviceQueue(_device, indices.graphicsFamily.value(), 0, &_graphicsQueue);
  vkGetDeviceQueue(_device, indices.presentFamily.value(), 0, &_presentQueue);
  createCommandPool();
}

ft::Device::~Device() {
  vkDestroyCommandPool(_device, _commandPool, nullptr);
  vkDestroyDevice(_device, nullptr);
}

VkDevice ft::Device::getVKDevice() const { return _device; }
VkQueue ft::Device::getVKGraphicsQueue() const { return _graphicsQueue; }
VkQueue ft::Device::getVKPresentQueue() const { return _presentQueue; }

uint32_t ft::Device::findMemoryType(uint32_t typeFilter,
                                    VkMemoryPropertyFlags properties) {
  VkPhysicalDeviceMemoryProperties deviceMemoryProperties =
      _ftPhysicalDevice->getPhysicalDeviceMemoryProperties();

  for (uint32_t i = 0; i < deviceMemoryProperties.memoryTypeCount; ++i) {
    if (typeFilter & (1 << i) &&
        (deviceMemoryProperties.memoryTypes[i].propertyFlags & properties) ==
            properties) {
      return i;
    }
  }

  throw std::runtime_error("failed to find suitable memory type!");
}

VkSampleCountFlagBits ft::Device::getMSAASamples() const {
  return _ftPhysicalDevice->getMSAASamples();
}

VkPhysicalDeviceProperties ft::Device::getVKPhysicalDeviceProperties() const {
  return _ftPhysicalDevice->getPhysicalDeviceProperties();
}

VkFormatProperties ft::Device::getVKFormatProperties(VkFormat &format) const {
  return _ftPhysicalDevice->getFormatProperties(format);
}

VkFormat
ft::Device::selectSupportedFormat(const std::vector<VkFormat> &candidates,
                                  VkImageTiling tiling,
                                  VkFormatFeatureFlags features) const {
  for (VkFormat format : candidates) {
    VkFormatProperties props = getVKFormatProperties(format);
    if (tiling == VK_IMAGE_TILING_LINEAR &&
        (props.linearTilingFeatures & features) == features) {
      return format;
    } else if (tiling == VK_IMAGE_TILING_OPTIMAL &&
               (props.optimalTilingFeatures & features) == features) {
      return format;
    }
  }
  throw std::runtime_error("failed to find a supported format!");
}

VkFormat ft::Device::findDepthFormat() const {
  return selectSupportedFormat(
      {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT,
       VK_FORMAT_D24_UNORM_S8_UINT},
      VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
}

bool ft::Device::hasStencilComponent(VkFormat format) const {
  return format == VK_FORMAT_D16_UNORM_S8_UINT ||
         format == VK_FORMAT_D24_UNORM_S8_UINT;
}

void ft::Device::createCommandPool() {
  QueueFamilyIndices queueFamilyIndices = getQueueFamilyIndices();
  VkCommandPoolCreateInfo poolCreateInfo{};
  poolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  poolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
  poolCreateInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

  if (vkCreateCommandPool(_device, &poolCreateInfo, nullptr, &_commandPool) !=
      VK_SUCCESS) {
    throw std::runtime_error("failed to create a command pool!");
  }
}

VkCommandPool ft::Device::getVKCommandPool() const { return _commandPool; }

ft::QueueFamilyIndices ft::Device::getQueueFamilyIndices() const {
  return _ftPhysicalDevice->getQueueFamilyIndices();
}
