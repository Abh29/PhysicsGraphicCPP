#ifndef FTGRAPHICS_FT_IMAGE_H
#define FTGRAPHICS_FT_IMAGE_H

#include "ft_headers.h"
#include "ft_device.h"
#include "ft_command.h"

namespace ft {

	class Image {
	public:
		using pointer = std::shared_ptr<Image>;

		Image(Device::pointer device,
			  uint32_t width, uint32_t height, uint32_t mipLevel,
			  VkSampleCountFlagBits sampleCount, VkFormat format,
			  VkImageTiling tiling, VkImageUsageFlags usage,
			  VkMemoryPropertyFlags properties, VkImageAspectFlags aspectFlags);

		~Image();

		[[nodiscard]] VkImage 			getVKImage() const;
		[[nodiscard]] VkImageView 		getVKImageView() const;
		static void transitionImageLayout(Device::pointer device, VkImage image, VkFormat format,
										  VkImageLayout oldLayout, VkImageLayout newLayout,
										  uint32_t mipLevels);
		void generateMipmaps(VkFormat imageFormat);
        [[nodiscard]] uint32_t getHeight() const;
        [[nodiscard]] uint32_t getWidth() const;

	private:

		Device::pointer					_ftDevice;
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

		Image::pointer build(Device::pointer &device);

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
		Image::pointer 					_ftImage;
		Device::pointer 				_ftDevice;
	};

}

#endif //FTGRAPHICS_FT_IMAGE_H
