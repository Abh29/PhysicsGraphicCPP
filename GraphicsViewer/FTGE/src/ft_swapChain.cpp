#include "../include.h"
#include "../includes/ft_swapChain.h"


ft::SwapChain::SwapChain(std::shared_ptr<PhysicalDevice> &physicalDevice,
						 std::shared_ptr<Device> &device,
						 std::shared_ptr<Surface> &surface,
						 uint32_t width, uint32_t height) :
						 _ftPhysicalDevice(physicalDevice),
						 _ftDevice(device),
						 _ftSurface(surface),
						 _width(width),
						 _height(height){

	SwapChainSupportDetails details = querySwapChainSupport(physicalDevice->getVKPhysicalDevice());

	VkSurfaceFormatKHR  surfaceFormatKhr = chooseSwapSurfaceFormat(details.formats);
	VkPresentModeKHR presentModeKhr = chooseSwapPresentMode(details.presentModes);
	VkExtent2D extent = chooseSwapExtent(details.capabilities);

	uint32_t imageCount = details.capabilities.minImageCount + 1;
	if (details.capabilities.maxImageCount > 0)
		imageCount = imageCount > details.capabilities.maxImageCount ? details.capabilities.maxImageCount : imageCount;


	VkSwapchainCreateInfoKHR swapchainCreateInfoKhr{};
	swapchainCreateInfoKhr.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swapchainCreateInfoKhr.surface = _ftSurface->getVKSurface();
	swapchainCreateInfoKhr.minImageCount = imageCount;
	swapchainCreateInfoKhr.imageFormat = surfaceFormatKhr.format;
	swapchainCreateInfoKhr.imageColorSpace = surfaceFormatKhr.colorSpace;
	swapchainCreateInfoKhr.imageExtent = extent;
	swapchainCreateInfoKhr.imageArrayLayers = 1;
	swapchainCreateInfoKhr.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	QueueFamilyIndices indices = _ftPhysicalDevice->getQueueFamilyIndices();
	uint32_t indices_v[] = {indices.graphicsFamily.value(), indices.presentFamily.value()};

	if (indices.graphicsFamily != indices.presentFamily) {
		swapchainCreateInfoKhr.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		swapchainCreateInfoKhr.queueFamilyIndexCount = 2;
		swapchainCreateInfoKhr.pQueueFamilyIndices = indices_v;
	} else {
		swapchainCreateInfoKhr.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		swapchainCreateInfoKhr.queueFamilyIndexCount = 0;
		swapchainCreateInfoKhr.pQueueFamilyIndices = nullptr;
	}

	swapchainCreateInfoKhr.preTransform = details.capabilities.currentTransform;
	swapchainCreateInfoKhr.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	swapchainCreateInfoKhr.presentMode = presentModeKhr;
	swapchainCreateInfoKhr.clipped = VK_TRUE;
	swapchainCreateInfoKhr.oldSwapchain = VK_NULL_HANDLE;

	if (vkCreateSwapchainKHR(_ftDevice->getVKDevice(), &swapchainCreateInfoKhr, nullptr, &_swapChain) != VK_SUCCESS) {
		throw std::runtime_error("failed to create a swapChain!");
	}

	uint32_t actualImageCount;
	vkGetSwapchainImagesKHR(_ftDevice->getVKDevice(), _swapChain, &actualImageCount, nullptr);
	_swapChainImages.resize(actualImageCount);
	vkGetSwapchainImagesKHR(_ftDevice->getVKDevice(), _swapChain, &actualImageCount, _swapChainImages.data());

	_swapChainImageFormat = surfaceFormatKhr.format;
	_swapChainExtent = extent;
}

ft::SwapChain::~SwapChain() {
	vkDestroySwapchainKHR(_ftDevice->getVKDevice(), _swapChain, nullptr);
}

VkSwapchainKHR ft::SwapChain::getVKSwapChain() const {return _swapChain;}

VkFormat ft::SwapChain::getVKSwapChainImageFormat() const {return _swapChainImageFormat;}

VkExtent2D ft::SwapChain::getVKSwapChainExtent() const {return _swapChainExtent;}

std::vector<VkImage> ft::SwapChain::getVKSwapChainImages() const {return _swapChainImages;}


ft::SwapChainSupportDetails ft::SwapChain::querySwapChainSupport(VkPhysicalDevice device) {
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

VkSurfaceFormatKHR ft::SwapChain::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
	for (const auto& af : availableFormats) {
		if (af.format == VK_FORMAT_B8G8R8A8_SRGB && af.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
			return af;
		}
	}
	return availableFormats[0];
}

VkPresentModeKHR ft::SwapChain::chooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes) {
	for (const auto& apm : availablePresentModes) {
		if (apm == VK_PRESENT_MODE_MAILBOX_KHR)
			return apm;
	}
	return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D ft::SwapChain::chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilitiesKhr) {
	if (capabilitiesKhr.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
		return capabilitiesKhr.currentExtent;
	}

	VkExtent2D actualExtent {_width, _height};

	actualExtent.width = std::clamp(actualExtent.width, capabilitiesKhr.minImageExtent.width, capabilitiesKhr.maxImageExtent.width);
	actualExtent.height = std::clamp(actualExtent.height, capabilitiesKhr.minImageExtent.height, capabilitiesKhr.maxImageExtent.height);

	return actualExtent;
}


