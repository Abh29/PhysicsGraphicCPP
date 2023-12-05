#ifndef FTGRAPHICS_FT_BUFFER_H
#define FTGRAPHICS_FT_BUFFER_H

#include "ft_headers.h"
#include "ft_device.h"

namespace ft {

	class Device;

	class Buffer {
	public:

		using pointer = std::shared_ptr<Buffer>;

		Buffer(Device::pointer &device, VkDeviceSize size,
			   VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags	memoryProperties,
			   bool mapped,  VkDeviceSize mappedOffset, VkMemoryMapFlags mappedFlags);

		~Buffer();


		[[nodiscard]] VkBuffer getVKBuffer() const;
		[[nodiscard]] void* getMappedData() const;
		void copyToMappedData(void *src, uint32_t size);
		bool isMapped() const;


	private:
		Device::pointer 					_ftDevice;
		VkDeviceSize 						_size;
		VkBuffer 							_buffer;
		VkDeviceMemory 						_bufferMemory;
		void*								_mappedData = nullptr;
		bool								_isMapped = false;
	};


	class BufferBuilder {

	public:
		BufferBuilder() = default;
		~BufferBuilder() = default;

		BufferBuilder& setSize(VkDeviceSize size);
		BufferBuilder& setUsageFlags(VkBufferUsageFlags usageFlags);
		BufferBuilder& setMemoryProperties(VkMemoryPropertyFlags memoryProperties);
		BufferBuilder& setIsMapped(bool isMapped);
		BufferBuilder& setMappedOffset(VkDeviceSize mappedOffset);
		BufferBuilder& setMappedFlags(VkMemoryMapFlags mappedFlags);
		Buffer::pointer build(Device::pointer &device);

	private:
		VkDeviceSize 						_size;
		VkBufferUsageFlags 					_usageFlags;
		VkMemoryPropertyFlags 				_memoryProperties;
		std::unique_ptr<Buffer>				_ftBuffer;
		bool 								_isMapped = false;
		VkDeviceSize 						_mappedOffset = 0;
		VkMemoryMapFlags 					_mappedFlags = 0;
	};

}

#endif //FTGRAPHICS_FT_BUFFER_H
