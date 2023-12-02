#ifndef FTGRAPHICS_FT_BUFFER_H
#define FTGRAPHICS_FT_BUFFER_H

#include "ft_headers.h"
#include "ft_device.h"

namespace ft {

	class Device;

	class Buffer {

	public:
		Buffer(std::shared_ptr<Device> &device, VkDeviceSize size,
			   VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags	memoryProperties);

		~Buffer();


		VkBuffer getVKBuffer() const;

	private:
		std::shared_ptr<Device>				_ftDevice;
		VkDeviceSize 						_size;
		VkBuffer 							_buffer;
		VkDeviceMemory 						_bufferMemory;
	};


	class BufferBuilder {

	public:
		BufferBuilder();
		~BufferBuilder() = default;

		BufferBuilder& setSize(VkDeviceSize size);
		BufferBuilder& setUsageFlags(VkBufferUsageFlags usageFlags);
		BufferBuilder& setMemoryProperties(VkMemoryPropertyFlags memoryProperties);
		std::unique_ptr<Buffer> build(std::shared_ptr<Device> &device);

	private:
		VkDeviceSize 						_size;
		VkBufferUsageFlags 					_usageFlags;
		VkMemoryPropertyFlags 				_memoryProperties;
		std::unique_ptr<Buffer>				_ftBuffer;
	};

}

#endif //FTGRAPHICS_FT_BUFFER_H
