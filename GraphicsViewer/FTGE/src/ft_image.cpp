#include "../include.h"

ft::Image::Image(std::shared_ptr<Device> &device,
uint32_t width, uint32_t height, uint32_t mipLevel,
VkSampleCountFlagBits sampleCount, VkFormat format,
VkImageTiling tiling, VkImageUsageFlags usage,
VkMemoryPropertyFlags properties, VkImageAspectFlags aspectFlags) :
_ftDevice(device), _width(width), _height(height),
_mipLevel(mipLevel), _sampleCount(sampleCount){

	// create an image on the device
	VkImageCreateInfo imageCreateInfo{};
	imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
	imageCreateInfo.extent.width = width;
	imageCreateInfo.extent.height = height;
	imageCreateInfo.extent.depth = 1;
	imageCreateInfo.mipLevels = mipLevel;
	imageCreateInfo.arrayLayers = 1;
	imageCreateInfo.format = format;
	imageCreateInfo.tiling = tiling;
	imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageCreateInfo.usage = usage;
	imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	imageCreateInfo.samples = sampleCount;
	imageCreateInfo.flags = 0;

	if (vkCreateImage(_ftDevice->getVKDevice(), &imageCreateInfo, nullptr, &_image) != VK_SUCCESS) {
		throw std::runtime_error("failed to create a texture image!");
	}

	// allocate memory on the device for the image
	VkMemoryRequirements memoryRequirements;
	vkGetImageMemoryRequirements(_ftDevice->getVKDevice(), _image, &memoryRequirements);

	VkMemoryAllocateInfo	memoryAllocateInfo{};
	memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memoryAllocateInfo.allocationSize = memoryRequirements.size;
	memoryAllocateInfo.memoryTypeIndex = _ftDevice->findMemoryType(memoryRequirements.memoryTypeBits, properties);

	if (vkAllocateMemory(_ftDevice->getVKDevice(), &memoryAllocateInfo, nullptr, &_imageMemory) != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate memory for the texture image!");
	}

	// bind the allocated memory to the image
	vkBindImageMemory(_ftDevice->getVKDevice(), _image, _imageMemory, 0);

	// create the image view
	VkImageViewCreateInfo imageViewCreateInfo{};
	imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	imageViewCreateInfo.image = _image;
	imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	imageViewCreateInfo.format = format;
	imageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
	imageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
	imageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
	imageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
	imageViewCreateInfo.subresourceRange.aspectMask = aspectFlags;
	imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
	imageViewCreateInfo.subresourceRange.levelCount = mipLevel;
	imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
	imageViewCreateInfo.subresourceRange.layerCount = 1;

	if (vkCreateImageView(_ftDevice->getVKDevice(), &imageViewCreateInfo, nullptr, &_imageView) != VK_SUCCESS) {
		throw std::runtime_error("failed to create a texture image view!");
	}

}


ft::Image::~Image() {
	vkDestroyImage(_ftDevice->getVKDevice(), _image, nullptr);
	vkFreeMemory(_ftDevice->getVKDevice(), _imageMemory, nullptr);
	vkDestroyImageView(_ftDevice->getVKDevice(), _imageView, nullptr);
}

VkImage ft::Image::getVKImage() const {return _image;}

VkImageView ft::Image::getVKImageView() const {return _imageView;}


/****************************** ImageBuilder *********************************/
ft::ImageBuilder::ImageBuilder() {
	_ftImage = nullptr;
}

ft::Image::pointer ft::ImageBuilder::build(std::shared_ptr<Device> &device) {
	_ftImage = std::make_unique<ft::Image>(device,
			_width, _height, _mipLevel, _sampleCount, _format,
			_tiling, _usageFlags, _memoryProperties, _aspectFlags);

	return std::move(_ftImage);
}


ft::ImageBuilder& ft::ImageBuilder::setWidthHeight(uint32_t width, uint32_t height) {
	_width = width;
	_height = height;
	return *this;
}

ft::ImageBuilder& ft::ImageBuilder::setMipLevel(uint32_t mipLevel) {
	_mipLevel = mipLevel;
	return *this;
}

ft::ImageBuilder& ft::ImageBuilder::setSampleCount(VkSampleCountFlagBits sampleCount) {
	_sampleCount = sampleCount;
	return *this;
}

ft::ImageBuilder& ft::ImageBuilder::setFormat(VkFormat format) {
	_format = format;
	return *this;
}

ft::ImageBuilder& ft::ImageBuilder::setTiling(VkImageTiling tiling) {
	_tiling = tiling;
	return *this;
}

ft::ImageBuilder& ft::ImageBuilder::setUsageFlags(VkImageUsageFlags usageFlags) {
	_usageFlags = usageFlags;
	return *this;
}

ft::ImageBuilder& ft::ImageBuilder::setMemoryProperties(VkMemoryPropertyFlags memoryProperties) {
	_memoryProperties = memoryProperties;
	return *this;
}

ft::ImageBuilder& ft::ImageBuilder::setAspectFlags(VkImageAspectFlags aspectFlags) {
	_aspectFlags = aspectFlags;
	return *this;
}
