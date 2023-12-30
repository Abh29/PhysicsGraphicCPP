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
	createColorResources();
	createDepthResources();
	createSyncObjects();
}

ft::SwapChain::~SwapChain() {
	for (auto fb : _frameBuffers) {
		vkDestroyFramebuffer(_ftDevice->getVKDevice(), fb, nullptr);
	}
	for (auto & _swapChainImageView : _swapChainImageViews) {
		vkDestroyImageView(_ftDevice->getVKDevice(), _swapChainImageView, nullptr);
	}

	for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
		vkDestroySemaphore(_ftDevice->getVKDevice(), _renderFinishedSemaphores[i], nullptr);
		vkDestroySemaphore(_ftDevice->getVKDevice(), _imageAvailableSemaphores[i], nullptr);
		vkDestroyFence(_ftDevice->getVKDevice(), _inFlightFences[i], nullptr);
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

float ft::SwapChain::getAspect() const {
	return (float) _swapChainExtent.width / (float) _swapChainExtent.height;
}

std::pair<VkResult, uint32_t> ft::SwapChain::acquireNextImage(VkFence fence) {
	// wait for previous frame
	vkWaitForFences(_ftDevice->getVKDevice(), 1, &_inFlightFences[_currentFrame], VK_TRUE, UINT64_MAX);
	vkResetFences(_ftDevice->getVKDevice(), 1, &_inFlightFences[_currentFrame]);
	VkResult result = vkAcquireNextImageKHR(_ftDevice->getVKDevice(), _swapChain, UINT64_MAX, _imageAvailableSemaphores[_currentFrame], fence, &_imageNext);
	if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
		throw std::runtime_error("failed to acquire the swap chain image!");
	}
	return {result, _imageNext};
}

VkPresentModeKHR ft::SwapChain::getPreferredPresentMode() const {
	return _preferredMode;
}

void ft::SwapChain::createColorResources() {
	ImageBuilder imageBuilder;
	_ftColorImage = imageBuilder.setWidthHeight(_width, _height)
			.setMipLevel(1)
			.setSampleCount(_ftDevice->getMSAASamples())
			.setFormat(_swapChainImageFormat)
			.setTiling(VK_IMAGE_TILING_OPTIMAL)
			.setUsageFlags(VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT)
			.setMemoryProperties(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
			.setAspectFlags(VK_IMAGE_ASPECT_COLOR_BIT)
			.build(_ftDevice);
}

void ft::SwapChain::createDepthResources() {
	ImageBuilder imageBuilder;
	VkFormat depthFormat = _ftDevice->findDepthFormat();
	_ftDepthImage = imageBuilder.setWidthHeight(_width, _height)
			.setMipLevel(1)
			.setSampleCount(_ftDevice->getMSAASamples())
			.setFormat(depthFormat)
			.setTiling(VK_IMAGE_TILING_OPTIMAL)
			.setUsageFlags(VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)
			.setMemoryProperties(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
			.setAspectFlags(VK_IMAGE_ASPECT_DEPTH_BIT)
			.build(_ftDevice);

	Image::transitionImageLayout(_ftDevice, _ftDepthImage->getVKImage(), depthFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, 1);
}

void ft::SwapChain::createFrameBuffers(ft::RenderPass::pointer renderPass) {
	_frameBuffers.resize(_swapChainImages.size());

	for (size_t i = 0; i < _swapChainImages.size(); ++i) {
		std::array<VkImageView, 3> attachments = {
				_ftColorImage->getVKImageView(),
				_ftDepthImage->getVKImageView(),
				_swapChainImageViews[i],
		};

		VkFramebufferCreateInfo framebufferCreateInfo{};
		framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferCreateInfo.renderPass = renderPass->getVKRenderPass();
		framebufferCreateInfo.attachmentCount = (uint32_t) attachments.size();
		framebufferCreateInfo.pAttachments = attachments.data();
		framebufferCreateInfo.width = _swapChainExtent.width;
		framebufferCreateInfo.height = _swapChainExtent.height;
		framebufferCreateInfo.layers = 1;

		if (vkCreateFramebuffer(_ftDevice->getVKDevice(), &framebufferCreateInfo, nullptr,
								&_frameBuffers[i]) != VK_SUCCESS) {
			throw std::runtime_error("failed to create a framebuffer!");
		}
	}
}

std::vector<VkFramebuffer> &ft::SwapChain::getFrameBuffers() {return _frameBuffers;}

// semaphores and fences
void ft::SwapChain::createSyncObjects() {
	_imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
	_renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
	_inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

	VkSemaphoreCreateInfo semaphoreCreateInfo{};
	semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkFenceCreateInfo fenceCreateInfo{};
	fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT; // this is for the first call

	for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
		if (vkCreateSemaphore(_ftDevice->getVKDevice(), &semaphoreCreateInfo, nullptr, &_imageAvailableSemaphores[i]) != VK_SUCCESS ||
			vkCreateSemaphore(_ftDevice->getVKDevice(), &semaphoreCreateInfo, nullptr, &_renderFinishedSemaphores[i]) != VK_SUCCESS ||
			vkCreateFence(_ftDevice->getVKDevice(), &fenceCreateInfo, nullptr, &_inFlightFences[i]) != VK_SUCCESS) {
			throw std::runtime_error("failed to create sync objects!");
		}
	}

}

VkResult ft::SwapChain::submit(ft::CommandBuffer::pointer commandBuffer, uint32_t imageIndex) {
	// submitting the command buffer
	VkSemaphore waitSemaphores[] = {_imageAvailableSemaphores[_currentFrame]};
	VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
	VkSemaphore signalSemaphores[] = {_renderFinishedSemaphores[_currentFrame]};

	VkSubmitInfo submitInfo{};
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitSemaphores;
	submitInfo.pWaitDstStageMask = waitStages;
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphores;

	commandBuffer->submit(_ftDevice->getVKGraphicsQueue(), submitInfo, _inFlightFences[_currentFrame]);

	// presentation
	VkSwapchainKHR	swapChains[] = {_swapChain};

	VkPresentInfoKHR presentInfoKhr{};
	presentInfoKhr.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfoKhr.waitSemaphoreCount = 1;
	presentInfoKhr.pWaitSemaphores = signalSemaphores;
	presentInfoKhr.swapchainCount = 1;
	presentInfoKhr.pSwapchains = swapChains;
	presentInfoKhr.pImageIndices = &imageIndex;
	presentInfoKhr.pResults = nullptr;

	VkResult result2 = vkQueuePresentKHR(_ftDevice->getVKPresentQueue(), &presentInfoKhr);

	_currentFrame = (_currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
	return result2;
}

