#include "../includes/ft_image.h"
#include <cstdint>
#include <vulkan/vulkan_core.h>

ft::Image::Image(ft::Device::pointer device, VkImageCreateInfo createInfo,
                 VkMemoryPropertyFlags properties,
                 VkImageAspectFlags aspectFlags, VkImageViewType viewType)
    : _ftDevice(std::move(device)), _createInfo(createInfo) {

  // create an image on the device
  _createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
  _mipLevel = _createInfo.mipLevels;
  _width = _createInfo.extent.width;
  _height = _createInfo.extent.height;
  if (vkCreateImage(_ftDevice->getVKDevice(), &_createInfo, nullptr, &_image) !=
      VK_SUCCESS) {
    throw std::runtime_error("failed to create a texture image!");
  }

  // allocate memory on the device for the image
  VkMemoryRequirements memoryRequirements;
  vkGetImageMemoryRequirements(_ftDevice->getVKDevice(), _image,
                               &memoryRequirements);

  VkMemoryAllocateInfo memoryAllocateInfo{};
  memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  memoryAllocateInfo.allocationSize = memoryRequirements.size;
  memoryAllocateInfo.memoryTypeIndex =
      _ftDevice->findMemoryType(memoryRequirements.memoryTypeBits, properties);

  if (vkAllocateMemory(_ftDevice->getVKDevice(), &memoryAllocateInfo, nullptr,
                       &_imageMemory) != VK_SUCCESS) {
    throw std::runtime_error(
        "failed to allocate memory for the texture image!");
  }

  // bind the allocated memory to the image
  vkBindImageMemory(_ftDevice->getVKDevice(), _image, _imageMemory, 0);

  // create the image view
  VkImageViewCreateInfo imageViewCreateInfo{};
  imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
  imageViewCreateInfo.image = _image;
  imageViewCreateInfo.viewType = viewType;
  imageViewCreateInfo.format = _createInfo.format;
  imageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
  imageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
  imageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
  imageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
  imageViewCreateInfo.subresourceRange.aspectMask = aspectFlags;
  imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
  imageViewCreateInfo.subresourceRange.levelCount = _mipLevel;
  imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
  imageViewCreateInfo.subresourceRange.layerCount = _createInfo.arrayLayers;

  if (vkCreateImageView(_ftDevice->getVKDevice(), &imageViewCreateInfo, nullptr,
                        &_imageView) != VK_SUCCESS) {
    throw std::runtime_error("failed to create a texture image view!");
  }
}

ft::Image::~Image() {
  vkDestroyImage(_ftDevice->getVKDevice(), _image, nullptr);
  vkFreeMemory(_ftDevice->getVKDevice(), _imageMemory, nullptr);
  vkDestroyImageView(_ftDevice->getVKDevice(), _imageView, nullptr);
}

VkImage ft::Image::getVKImage() const { return _image; }

VkImageView ft::Image::getVKImageView() const { return _imageView; }

void ft::Image::transitionImageLayout(ft::Device::pointer device, VkImage image,
                                      VkFormat format, VkImageLayout oldLayout,
                                      VkImageLayout newLayout,
                                      uint32_t mipLevels) {

  // create a command buffer for transition
  std::unique_ptr<CommandBuffer> commandBuffer =
      std::make_unique<CommandBuffer>(device);
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

  if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED &&
      newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
  } else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
             newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
  } else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED &&
             newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
    // set the right aspectMask
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    barrier.subresourceRange.aspectMask |= device->hasStencilComponent(format)
                                               ? VK_IMAGE_ASPECT_STENCIL_BIT
                                               : VK_IMAGE_ASPECT_NONE;
    barrier.srcAccessMask = VK_ACCESS_NONE;
    barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
                            VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
  } else if (oldLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL &&
             newLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL) {
    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
  } else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL &&
             newLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) {
    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = 0;
    sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    destinationStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
  } else {
    throw std::invalid_argument("unsupported layout transition!");
  }

  // submit the barrier
  vkCmdPipelineBarrier(commandBuffer->getVKCommandBuffer(), sourceStage,
                       destinationStage, 0, 0, nullptr, 0, nullptr, 1,
                       &barrier);

  // execution and clean up
  commandBuffer->end();
  commandBuffer->submit(device->getVKGraphicsQueue());
}

void ft::Image::transitionLayout2(VkImageLayout oldLayout,
                                  VkImageLayout newLayout,
                                  VkImageSubresourceRange subresourceRange,
                                  VkPipelineStageFlags srcStageMask,
                                  VkPipelineStageFlags dstStageMask) {

  // create a command buffer for transition
  std::unique_ptr<CommandBuffer> commandBuffer =
      std::make_unique<CommandBuffer>(_ftDevice);
  commandBuffer->beginRecording(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

  // Create an image barrier object
  VkImageMemoryBarrier imageMemoryBarrier = {};
  imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
  imageMemoryBarrier.oldLayout = oldLayout;
  imageMemoryBarrier.newLayout = newLayout;
  imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  imageMemoryBarrier.image = _image;
  imageMemoryBarrier.subresourceRange = subresourceRange;

  // Source layouts (old)
  // Source access mask controls actions that have to be finished on the old
  // layout before it will be transitioned to the new layout
  switch (oldLayout) {
  case VK_IMAGE_LAYOUT_UNDEFINED:
    // Image layout is undefined (or does not matter)
    // Only valid as initial layout
    // No flags required, listed only for completeness
    imageMemoryBarrier.srcAccessMask = 0;
    break;

  case VK_IMAGE_LAYOUT_PREINITIALIZED:
    // Image is preinitialized
    // Only valid as initial layout for linear images, preserves memory contents
    // Make sure host writes have been finished
    imageMemoryBarrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
    break;

  case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
    // Image is a color attachment
    // Make sure any writes to the color buffer have been finished
    imageMemoryBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    break;

  case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
    // Image is a depth/stencil attachment
    // Make sure any writes to the depth/stencil buffer have been finished
    imageMemoryBarrier.srcAccessMask =
        VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    break;

  case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
    // Image is a transfer source
    // Make sure any reads from the image have been finished
    imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
    break;

  case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
    // Image is a transfer destination
    // Make sure any writes to the image have been finished
    imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    break;

  case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
    // Image is read by a shader
    // Make sure any shader reads from the image have been finished
    imageMemoryBarrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
    break;
  default:
    // Other source layouts aren't handled (yet)
    break;
  }

  // Target layouts (new)
  // Destination access mask controls the dependency for the new image layout
  switch (newLayout) {
  case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
    // Image will be used as a transfer destination
    // Make sure any writes to the image have been finished
    imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    break;

  case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
    // Image will be used as a transfer source
    // Make sure any reads from the image have been finished
    imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
    break;

  case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
    // Image will be used as a color attachment
    // Make sure any writes to the color buffer have been finished
    imageMemoryBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    break;

  case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
    // Image layout will be used as a depth/stencil attachment
    // Make sure any writes to depth/stencil buffer have been finished
    imageMemoryBarrier.dstAccessMask =
        imageMemoryBarrier.dstAccessMask |
        VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    break;

  case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
    // Image will be read in a shader (sampler, input attachment)
    // Make sure any writes to the image have been finished
    if (imageMemoryBarrier.srcAccessMask == 0) {
      imageMemoryBarrier.srcAccessMask =
          VK_ACCESS_HOST_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT;
    }
    imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    break;
  default:
    // Other source layouts aren't handled (yet)
    // todo: handle this case
    break;
  }

  // submit the barrier
  vkCmdPipelineBarrier(commandBuffer->getVKCommandBuffer(), srcStageMask,
                       dstStageMask, 0, 0, nullptr, 0, nullptr, 1,
                       &imageMemoryBarrier);

  // execution and clean up
  commandBuffer->end();
  commandBuffer->submit(_ftDevice->getVKGraphicsQueue());
  _imageLayout = newLayout;
}

void ft::Image::transitionLayout(VkFormat format, VkImageLayout layout) {
  // create a command buffer for transition
  std::unique_ptr<CommandBuffer> commandBuffer =
      std::make_unique<CommandBuffer>(_ftDevice);
  commandBuffer->beginRecording(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

  // create a barrier for sync
  VkImageMemoryBarrier barrier{};
  barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
  barrier.oldLayout = _imageLayout;
  barrier.newLayout = layout;
  barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  barrier.image = _image;
  barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  barrier.subresourceRange.baseMipLevel = 0;
  barrier.subresourceRange.levelCount = _mipLevel;
  barrier.subresourceRange.baseArrayLayer = 0;
  barrier.subresourceRange.layerCount = 1;

  // transition barrier masks
  VkPipelineStageFlags sourceStage;
  VkPipelineStageFlags destinationStage;

  if (_imageLayout == VK_IMAGE_LAYOUT_UNDEFINED &&
      layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
  } else if (_imageLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
             layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
  } else if (_imageLayout == VK_IMAGE_LAYOUT_UNDEFINED &&
             layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
    // set the right aspectMask
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    barrier.subresourceRange.aspectMask |=
        _ftDevice->hasStencilComponent(format) ? VK_IMAGE_ASPECT_STENCIL_BIT
                                               : VK_IMAGE_ASPECT_NONE;
    barrier.srcAccessMask = VK_ACCESS_NONE;
    barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
                            VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
  } else if (_imageLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL &&
             layout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL) {
    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
  } else if (_imageLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL &&
             layout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) {
    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = 0;
    sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    destinationStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
  } else {
    throw std::invalid_argument("unsupported layout transition!");
  }

  // submit the barrier
  vkCmdPipelineBarrier(commandBuffer->getVKCommandBuffer(), sourceStage,
                       destinationStage, 0, 0, nullptr, 0, nullptr, 1,
                       &barrier);

  // execution and clean up
  commandBuffer->end();
  commandBuffer->submit(_ftDevice->getVKGraphicsQueue());
  _imageLayout = layout;
}

void ft::Image::generateMipmaps(VkFormat imageFormat) {
  // check if the image format supports linear blitting
  VkFormatProperties formatProperties =
      _ftDevice->getVKFormatProperties(imageFormat);

  if (!(formatProperties.optimalTilingFeatures &
        VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT)) {
    throw std::runtime_error(
        "texture image format does not support linear blitting!");
  }

  std::unique_ptr<CommandBuffer> commandBuffer =
      std::make_unique<CommandBuffer>(_ftDevice);
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

    vkCmdPipelineBarrier(
        commandBuffer->getVKCommandBuffer(), VK_PIPELINE_STAGE_TRANSFER_BIT,
        VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);

    VkImageBlit imageBlit{};
    imageBlit.srcOffsets[0] = {0, 0, 0};
    imageBlit.srcOffsets[1] = {mipWidth, mipHeight, 1};
    imageBlit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    imageBlit.srcSubresource.mipLevel = i - 1;
    imageBlit.srcSubresource.baseArrayLayer = 0;
    imageBlit.srcSubresource.layerCount = 1;
    imageBlit.dstOffsets[0] = {0, 0, 0};
    imageBlit.dstOffsets[1] = {mipWidth > 1 ? mipWidth / 2 : 1,
                               mipHeight > 1 ? mipHeight / 2 : 1, 1};
    imageBlit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    imageBlit.dstSubresource.mipLevel = i;
    imageBlit.dstSubresource.baseArrayLayer = 0;
    imageBlit.dstSubresource.layerCount = 1;

    vkCmdBlitImage(commandBuffer->getVKCommandBuffer(), _image,
                   VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, _image,
                   VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &imageBlit,
                   VK_FILTER_LINEAR);

    barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

    vkCmdPipelineBarrier(commandBuffer->getVKCommandBuffer(),
                         VK_PIPELINE_STAGE_TRANSFER_BIT,
                         VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr,
                         0, nullptr, 1, &barrier);

    if (mipWidth > 1)
      mipWidth /= 2;
    if (mipHeight > 1)
      mipHeight /= 2;
  }

  barrier.subresourceRange.baseMipLevel = _mipLevel - 1;
  barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
  barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
  barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
  barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

  vkCmdPipelineBarrier(commandBuffer->getVKCommandBuffer(),
                       VK_PIPELINE_STAGE_TRANSFER_BIT,
                       VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0,
                       nullptr, 1, &barrier);

  commandBuffer->end();
  commandBuffer->submit(_ftDevice->getVKGraphicsQueue());
}

uint32_t ft::Image::getWidth() const { return _width; }

uint32_t ft::Image::getHeight() const { return _height; }

/****************************** ImageBuilder *********************************/
ft::ImageBuilder::ImageBuilder() {
  _ftImage = nullptr;
  _createInfo = {};
  _createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
  _createInfo.imageType = VK_IMAGE_TYPE_2D;
  _createInfo.extent.depth = 1;
  _createInfo.arrayLayers = 1;
  _createInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
}

ft::Image::pointer ft::ImageBuilder::build(std::shared_ptr<Device> &device) {
  _ftImage = std::make_unique<ft::Image>(device, _createInfo, _memoryProperties,
                                         _aspectFlags, _imageViewType);
  return std::move(_ftImage);
}

ft::ImageBuilder &ft::ImageBuilder::setWidthHeight(uint32_t width,
                                                   uint32_t height,
                                                   uint32_t depth) {
  _createInfo.extent = {width, height, depth};
  return *this;
}

ft::ImageBuilder &ft::ImageBuilder::setMipLevel(uint32_t mipLevel) {
  _createInfo.mipLevels = mipLevel;
  return *this;
}

ft::ImageBuilder &
ft::ImageBuilder::setSampleCount(VkSampleCountFlagBits sampleCount) {
  _createInfo.samples = sampleCount;
  return *this;
}

ft::ImageBuilder &ft::ImageBuilder::setFormat(VkFormat format) {
  _createInfo.format = format;
  return *this;
}

ft::ImageBuilder &ft::ImageBuilder::setTiling(VkImageTiling tiling) {
  _createInfo.tiling = tiling;
  return *this;
}

ft::ImageBuilder &
ft::ImageBuilder::setUsageFlags(VkImageUsageFlags usageFlags) {
  _createInfo.usage = usageFlags;
  return *this;
}

ft::ImageBuilder &
ft::ImageBuilder::setMemoryProperties(VkMemoryPropertyFlags memoryProperties) {
  _memoryProperties = memoryProperties;
  return *this;
}

ft::ImageBuilder &
ft::ImageBuilder::setAspectFlags(VkImageAspectFlags aspectFlags) {
  _aspectFlags = aspectFlags;
  return *this;
}

ft::ImageBuilder &ft::ImageBuilder::setLayout(VkImageLayout imageLayout) {
  //_createInfo.initialLayout = imageLayout;
  (void)imageLayout;
  return *this;
}

ft::ImageBuilder &ft::ImageBuilder::setCreateFlags(VkImageCreateFlags flags) {
  _createInfo.flags = flags;
  return *this;
}

ft::ImageBuilder &ft::ImageBuilder::setArrayLayers(uint32_t layers) {
  _createInfo.arrayLayers = layers;
  return *this;
}

ft::ImageBuilder &ft::ImageBuilder::setViewType(VkImageViewType viewType) {
  _imageViewType = viewType;
  return *this;
}

