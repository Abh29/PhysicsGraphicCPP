#include "../includes/ft_image.h"

ft::Image::Image(ft::Device::pointer device,
uint32_t width, uint32_t height, uint32_t mipLevel,
VkSampleCountFlagBits sampleCount, VkFormat format,
VkImageTiling tiling, VkImageUsageFlags usage,
VkMemoryPropertyFlags properties, VkImageAspectFlags aspectFlags) :
_ftDevice(std::move(device)), _width(width), _height(height),
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

void ft::Image::transitionImageLayout(ft::Device::pointer device, VkImage image, VkFormat format,
									  VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels) {

	// create a command buffer for transition
	std::unique_ptr<CommandBuffer>	commandBuffer = std::make_unique<CommandBuffer>(device);
	commandBuffer->beginRecording(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);


	// create a barrier for sync
	VkImageMemoryBarrier barrier{};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.oldLayout = oldLayout;
	barrier.newLayout = newLayout;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.image = image;
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = mipLevels;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;

	// transition barrier masks
	VkPipelineStageFlags sourceStage;
	VkPipelineStageFlags destinationStage;

	if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	} else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	} else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
		// set the right aspectMask
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
		barrier.subresourceRange.aspectMask |= device->hasStencilComponent(format) ? VK_IMAGE_ASPECT_STENCIL_BIT : VK_IMAGE_ASPECT_NONE;
		barrier.srcAccessMask = VK_ACCESS_NONE;
		barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
								VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	}else {
		throw std::invalid_argument("unsopported layout transition!");
	}

	// submit the barrier
	vkCmdPipelineBarrier(commandBuffer->getVKCommandBuffer(),
						 sourceStage, destinationStage,
						 0, 0, nullptr, 0, nullptr,
						 1,&barrier);

	// execution and clean up
	commandBuffer->end();
	commandBuffer->submit(device->getVKGraphicsQueue());
}

void ft::Image::generateMipmaps(VkFormat imageFormat) {
	// check if the image format supports linear blitting
	VkFormatProperties formatProperties = _ftDevice->getVKFormatProperties(imageFormat);

	if (!(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT)) {
		throw std::runtime_error("texture image format does not support linear blitting!");
	}

	std::unique_ptr<CommandBuffer>	commandBuffer = std::make_unique<CommandBuffer>(_ftDevice);
	commandBuffer->beginRecording(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);


	VkImageMemoryBarrier barrier{};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.image = _image;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.levelCount = 1;
	barrier.subresourceRange.layerCount = 1;


	int32_t mipWidth = _width;
	int32_t mipHeight = _height;

	for (uint32_t i = 1; i < _mipLevel; ++i) {
		barrier.subresourceRange.baseMipLevel = i - 1;
		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

		vkCmdPipelineBarrier(commandBuffer->getVKCommandBuffer(), VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
							 0, nullptr, 0, nullptr, 1, &barrier);


		VkImageBlit imageBlit{};
		imageBlit.srcOffsets[0] = {0, 0, 0};
		imageBlit.srcOffsets[1] = {mipWidth, mipHeight, 1};
		imageBlit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		imageBlit.srcSubresource.mipLevel = i - 1;
		imageBlit.srcSubresource.baseArrayLayer = 0;
		imageBlit.srcSubresource.layerCount = 1;
		imageBlit.dstOffsets[0] = {0, 0, 0};
		imageBlit.dstOffsets[1] = {mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1};
		imageBlit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		imageBlit.dstSubresource.mipLevel = i;
		imageBlit.dstSubresource.baseArrayLayer = 0;
		imageBlit.dstSubresource.layerCount = 1;

		vkCmdBlitImage(commandBuffer->getVKCommandBuffer(), _image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, _image,
					   VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &imageBlit, VK_FILTER_LINEAR);


		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		vkCmdPipelineBarrier(commandBuffer->getVKCommandBuffer(), VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
							 0, nullptr, 0, nullptr, 1, &barrier);

		if (mipWidth > 1) mipWidth /= 2;
		if (mipHeight > 1) mipHeight /= 2;
	}


	barrier.subresourceRange.baseMipLevel = _mipLevel - 1;
	barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

	vkCmdPipelineBarrier(commandBuffer->getVKCommandBuffer(), VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
						 0, nullptr, 0, nullptr, 1, &barrier);

	commandBuffer->end();
	commandBuffer->submit(_ftDevice->getVKGraphicsQueue());
}

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
