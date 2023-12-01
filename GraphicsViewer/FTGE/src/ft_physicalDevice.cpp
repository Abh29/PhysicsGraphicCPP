#include "../include.h"

ft::PhysicalDevice::PhysicalDevice(
		std::shared_ptr<Instance> &instance,
		std::shared_ptr<Surface> &surface,
		std::vector<const char *> &deviceExtensions) :
_ftInstance(instance),
_ftSurface(surface),
_deviceExtensions(deviceExtensions)
{
	pickPhysicalDevice();
}

ft::PhysicalDevice::~PhysicalDevice() {

}

VkPhysicalDevice ft::PhysicalDevice::getVKPhysicalDevice() const {return _physicalDevice;}

VkSampleCountFlagBits ft::PhysicalDevice::getMSAASamples() const {return _msaaSamples;}

ft::PhysicalDevice::QueueFamilyIndices ft::PhysicalDevice::getQueueFamilyIndices() const {return _queueFamilyIndices;}

void ft::PhysicalDevice::pickPhysicalDevice() {

	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(_ftInstance->getVKInstance(), &deviceCount, nullptr);

	if (deviceCount == 0) {
		throw std::runtime_error("failed to find GPUs with Vulkan support!");
	}

	std::vector<VkPhysicalDevice> devices(deviceCount);
	vkEnumeratePhysicalDevices(_ftInstance->getVKInstance(), &deviceCount, devices.data());

	std::multimap<uint32_t , VkPhysicalDevice> candidates;

	for (const auto& device : devices) {
		int score = rateDeviceSuitability(device);
		candidates.insert(std::make_pair(score, device));
	}

	// Check if the best candidate is suitable at all
	if (candidates.rbegin()->first > 0) {
		_physicalDevice = candidates.rbegin()->second;
		_msaaSamples = getMaxUsableSampleCount();
	}

	if (_physicalDevice == VK_NULL_HANDLE) {
		throw std::runtime_error("failed to find a suitable GPU!");
	}
}

bool ft::PhysicalDevice::isDeviceSuitable(VkPhysicalDevice device) {
	QueueFamilyIndices indices = findQueueFamilies(device);
	if (!indices.isComplete()) return false;

	bool extensionsSupport = checkDeviceExtensionSupport(device);
	if (!extensionsSupport) return false;

	SwapChainSupportDetails details = querySwapChainSupport(device);
	if (details.formats.empty() || details.presentModes.empty()) return false;

	// check for sampler anisotropy support
	VkPhysicalDeviceFeatures deviceFeatures{};
	vkGetPhysicalDeviceFeatures(device, &deviceFeatures);
	if (deviceFeatures.samplerAnisotropy == VK_FALSE) return false;

	// print which suitable device found
	VkPhysicalDeviceProperties deviceProperties;
	vkGetPhysicalDeviceProperties(device, &deviceProperties);
	std::cout << "suitable device: " << deviceProperties.deviceName << std::endl;

	return true;
}

uint32_t ft::PhysicalDevice::rateDeviceSuitability(VkPhysicalDevice device) {
	int score = 0;

	VkPhysicalDeviceProperties deviceProperties;
	VkPhysicalDeviceFeatures deviceFeatures;

	vkGetPhysicalDeviceProperties(device, &deviceProperties);
	vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

	// Discrete GPUs have a significant performance advantage
	if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
		score += 1000;
	}

	// Maximum possible size of textures affects graphics quality
	score += deviceProperties.limits.maxImageDimension2D;

	// Application can't function without geometry shaders
	if (!deviceFeatures.geometryShader || !isDeviceSuitable(device)) {
		return 0;
	}

	return score;
}

VkSampleCountFlagBits ft::PhysicalDevice::getMaxUsableSampleCount() {
	VkPhysicalDeviceProperties physicalDeviceProperties{};
	vkGetPhysicalDeviceProperties(_physicalDevice, &physicalDeviceProperties);

	VkSampleCountFlags counts = physicalDeviceProperties.limits.framebufferColorSampleCounts & physicalDeviceProperties.limits.framebufferDepthSampleCounts;
	if (counts & VK_SAMPLE_COUNT_64_BIT) return VK_SAMPLE_COUNT_64_BIT;
	if (counts & VK_SAMPLE_COUNT_32_BIT) return VK_SAMPLE_COUNT_32_BIT;
	if (counts & VK_SAMPLE_COUNT_16_BIT) return VK_SAMPLE_COUNT_16_BIT;
	if (counts & VK_SAMPLE_COUNT_8_BIT) return VK_SAMPLE_COUNT_8_BIT;
	if (counts & VK_SAMPLE_COUNT_4_BIT) return VK_SAMPLE_COUNT_4_BIT;
	if (counts & VK_SAMPLE_COUNT_2_BIT) return VK_SAMPLE_COUNT_2_BIT;

	return VK_SAMPLE_COUNT_1_BIT;
}

bool ft::PhysicalDevice::checkDeviceExtensionSupport(VkPhysicalDevice device) {
	uint32_t extensionCount;
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

	std::vector<VkExtensionProperties> availableExtensions(extensionCount);
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

	std::set<std::string> requiredExtensions(_deviceExtensions.begin(), _deviceExtensions.end());
	for (const auto& extension : availableExtensions) {
		requiredExtensions.erase(extension.extensionName);
	}

	return requiredExtensions.empty();
}

ft::PhysicalDevice::QueueFamilyIndices ft::PhysicalDevice::findQueueFamilies(VkPhysicalDevice device) {

	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

	int i = 0;
	for (const auto& queueFamily : queueFamilies) {
		if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
			_queueFamilyIndices.graphicsFamily = i;
		}

		VkBool32 presentSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(device, i, _ftSurface->getVKSurface(), &presentSupport);

		if (presentSupport) {
			_queueFamilyIndices.presentFamily = i;
		}

		if (_queueFamilyIndices.isComplete()) {
			break;
		}
		i++;
	}

	return _queueFamilyIndices;
}

ft::PhysicalDevice::SwapChainSupportDetails ft::PhysicalDevice::querySwapChainSupport(VkPhysicalDevice device) {
	SwapChainSupportDetails details;

	// querying surface capabilities
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, _ftSurface->getVKSurface(), &details.capabilities);

	// querying supported surface formats
	uint32_t formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, _ftSurface->getVKSurface(), &formatCount, nullptr);
	if (formatCount != 0) {
		details.formats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, _ftSurface->getVKSurface(), &formatCount, details.formats.data());
	}

	// querying supported presentation
	uint32_t presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, _ftSurface->getVKSurface(), &presentModeCount, nullptr);
	if (presentModeCount != 0) {
		details.presentModes.resize(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, _ftSurface->getVKSurface(), &presentModeCount, details.presentModes.data());
	}

	return details;
}


bool ft::PhysicalDevice::QueueFamilyIndices::isComplete() const {
	return graphicsFamily.has_value() && presentFamily.has_value();
}