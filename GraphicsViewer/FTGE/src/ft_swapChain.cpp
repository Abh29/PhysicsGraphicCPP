#include "../include.h"
#include "../includes/ft_swapChain.h"


ft::SwapChain::SwapChain(std::shared_ptr<PhysicalDevice> &physicalDevice,
						 std::shared_ptr<Device> &device,
						 std::shared_ptr<Surface> &surface,
						 uint32_t width, uint32_t height,
						 VkPresentModeKHR preferredMode) :
						 _ftPhysicalDevice(physicalDevice),
						 _ftDevice(device),
						 _ftSurface(surface),
						 _preferredMode(preferredMode),
						 _width(width), _height(height){

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

	createImageViews();
}

ft::SwapChain::~SwapChain() {
	for (auto & _swapChainImageView : _swapChainImageViews) {
		vkDestroyImageView(_ftDevice->getVKDevice(), _swapChainImageView, nullptr);
	}
	vkDestroySwapchainKHR(_ftDevice->getVKDevice(), _swapChain, nullptr);
}

VkSwapchainKHR ft::SwapChain::getVKSwapChain() const {return _swapChain;}

VkFormat ft::SwapChain::getVKSwapChainImageFormat() const {return _swapChainImageFormat;}

VkExtent2D ft::SwapChain::getVKSwapChainExtent() const {return _swapChainExtent;}

std::vector<VkImage> ft::SwapChain::getVKSwapChainImages() const {return _swapChainImages;}

std::vector<VkImageView> ft::SwapChain::getVKSwapChainImageViews() const {return _swapChainImageViews;}


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
		if (apm == _preferredMode)
			return apm;
	}
	std::cout << "the preferred present mode is not supported, reverting to fifo" << std::endl;
	return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D ft::SwapChain::chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilitiesKhr) {
	if (capabilitiesKhr.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
		return capabilitiesKhr.currentExtent;
	}

	_width= std::clamp(_width, capabilitiesKhr.minImageExtent.width, capabilitiesKhr.maxImageExtent.width);
	_height = std::clamp(_height, capabilitiesKhr.minImageExtent.height, capabilitiesKhr.maxImageExtent.height);

	VkExtent2D actualExtent {_width, _height};

	return actualExtent;
}

void ft::SwapChain::createImageViews() {

	_swapChainImageViews.resize(_swapChainImages.size());

	int i = 0;
	for (auto & _swapChainImage : _swapChainImages) {
		VkImageViewCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		createInfo.image = _swapChainImage;
		createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		createInfo.format = _swapChainImageFormat;
		createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		createInfo.subresourceRange.baseMipLevel = 0;
		createInfo.subresourceRange.levelCount = 1;
		createInfo.subresourceRange.baseArrayLayer = 0;
		createInfo.subresourceRange.layerCount = 1;
		if (vkCreateImageView(_ftDevice->getVKDevice(), &createInfo, nullptr, &_swapChainImageViews[i]) != VK_SUCCESS) {
			throw std::runtime_error("failed to create an image view!");
		}
		++i;
	}
}

uint32_t ft::SwapChain::getWidth() const {
	return _width;
}

uint32_t ft::SwapChain::getHeight() const {
	return _height;
}

std::pair<VkResult, uint32_t> ft::SwapChain::acquireNextImage(VkSemaphore semaphore, VkFence fence) {
	VkResult result = vkAcquireNextImageKHR(_ftDevice->getVKDevice(), _swapChain, UINT64_MAX, semaphore, fence, &_imageNext);
	if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
		throw std::runtime_error("failed to acquire the swap chain image!");
	}
	return {result, _imageNext};
}

VkPresentModeKHR ft::SwapChain::getPreferredPresentMode() const {
	return _preferredMode;
}


