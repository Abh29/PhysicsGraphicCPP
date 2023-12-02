#ifndef FTGRAPHICS_FT_IMAGE_H
#define FTGRAPHICS_FT_IMAGE_H

#include "ft_headers.h"
#include "ft_device.h"

namespace ft {

	class Device;

	class Image {
	public:
		Image(std::shared_ptr<Device> &device,
			  uint32_t width, uint32_t height, uint32_t mipLevel,
			  VkSampleCountFlagBits sampleCount, VkFormat format,
			  VkImageTiling tiling, VkImageUsageFlags usage,
			  VkMemoryPropertyFlags properties, VkImageAspectFlags aspectFlags);

		~Image();


		VkImage 			getVKImage() const;
		VkImageView 		getVKImageView() const;

	private:

		std::shared_ptr<Device>			_ftDevice;
		uint32_t 						_width;
		uint32_t 						_height;
		uint32_t 						_mipLevel;
		VkSampleCountFlagBits			_sampleCount;
		VkImage 						_image;
		VkImageView 					_imageView;
		VkDeviceMemory 					_imageMemory;
	};



	class ImageBuilder {

	public:
		ImageBuilder();
		~ImageBuilder() = default;

		ImageBuilder& setWidthHeight(uint32_t width, uint32_t height);
		ImageBuilder& setMipLevel(uint32_t mipLevel);
		ImageBuilder& setSampleCount(VkSampleCountFlagBits sampleCount);
		ImageBuilder& setFormat(VkFormat format);
		ImageBuilder& setTiling(VkImageTiling tiling);
		ImageBuilder& setUsageFlags(VkImageUsageFlags usageFlags);
		ImageBuilder& setMemoryProperties(VkMemoryPropertyFlags memoryProperties);
		ImageBuilder& setAspectFlags(VkImageAspectFlags aspectFlags);

		std::unique_ptr<Image> build(std::shared_ptr<Device> &device);

	private:
		uint32_t 						_width;
		uint32_t 						_height;
		uint32_t 						_mipLevel;
		VkSampleCountFlagBits			_sampleCount;
		VkFormat						_format;
		VkImageTiling					_tiling;
		VkImageUsageFlags 				_usageFlags;
		VkMemoryPropertyFlags 			_memoryProperties;
		VkImageAspectFlags 				_aspectFlags;
		std::unique_ptr<Image>			_ftImage;
		std::shared_ptr<Device>			_ftDevice;
	};

}

#endif //FTGRAPHICS_FT_IMAGE_H
