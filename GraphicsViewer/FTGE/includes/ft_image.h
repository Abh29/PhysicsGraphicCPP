#ifndef FTGRAPHICS_FT_IMAGE_H
#define FTGRAPHICS_FT_IMAGE_H

#include "ft_command.h"
#include "ft_device.h"
#include "ft_headers.h"
#include <vulkan/vulkan_core.h>

namespace ft {

class Image {
public:
  using pointer = std::shared_ptr<Image>;

  Image(Device::pointer device, VkImageCreateInfo createInfo,
        VkMemoryPropertyFlags properties, VkImageAspectFlags aspectFlags);

  ~Image();

  [[nodiscard]] VkImage getVKImage() const;
  [[nodiscard]] VkImageView getVKImageView() const;
  static void transitionImageLayout(Device::pointer device, VkImage image,
                                    VkFormat format, VkImageLayout oldLayout,
                                    VkImageLayout newLayout,
                                    uint32_t mipLevels);
  void transitionLayout(VkFormat format, VkImageLayout layout);
  void generateMipmaps(VkFormat imageFormat);
  [[nodiscard]] uint32_t getHeight() const;
  [[nodiscard]] uint32_t getWidth() const;

private:
  Device::pointer _ftDevice;
  uint32_t _width;
  uint32_t _height;
  uint32_t _mipLevel;
  VkSampleCountFlagBits _sampleCount;
  VkImage _image;
  VkImageView _imageView;
  VkDeviceMemory _imageMemory;
  VkImageLayout _imageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  VkImageCreateInfo _createInfo;
};

class ImageBuilder {

public:
  ImageBuilder();
  ~ImageBuilder() = default;

  ImageBuilder &setWidthHeight(uint32_t width, uint32_t height,
                               uint32_t depth = 1);
  ImageBuilder &setMipLevel(uint32_t mipLevel);
  ImageBuilder &setSampleCount(VkSampleCountFlagBits sampleCount);
  ImageBuilder &setFormat(VkFormat format);
  ImageBuilder &setTiling(VkImageTiling tiling);
  ImageBuilder &setUsageFlags(VkImageUsageFlags usageFlags);
  ImageBuilder &setMemoryProperties(VkMemoryPropertyFlags memoryProperties);
  ImageBuilder &setAspectFlags(VkImageAspectFlags aspectFlags);
  ImageBuilder &setLayout(VkImageLayout imageLayout);
  ImageBuilder &setCreateFlags(VkImageCreateFlags flags);

  Image::pointer build(Device::pointer &device);

private:
  VkImageCreateInfo _createInfo;
  VkMemoryPropertyFlags _memoryProperties;
  VkImageAspectFlags _aspectFlags;
  Image::pointer _ftImage;
  Device::pointer _ftDevice;
};

} // namespace ft

#endif // FTGRAPHICS_FT_IMAGE_H
