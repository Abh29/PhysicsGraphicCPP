#include "../include.h"
#include "../includes/ft_buffer.h"


ft::Buffer::Buffer(std::shared_ptr<Device> &device, VkDeviceSize size, VkBufferUsageFlags usageFlags,
				   VkMemoryPropertyFlags memoryProperties) :
				   _ftDevice(device), _size(size)
				   {
						// create the buffer
						VkBufferCreateInfo bufferCreateInfo{};
						bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
						bufferCreateInfo.size = size;
						bufferCreateInfo.usage = usageFlags;
						bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

						if (vkCreateBuffer(_ftDevice->getVKDevice(), &bufferCreateInfo, nullptr, &_buffer) != VK_SUCCESS) {
							throw std::runtime_error("failed to create a vertex buffer!");
						}

						// query the device memory requirements for the buffer
						VkMemoryRequirements memoryRequirements;
						vkGetBufferMemoryRequirements(_ftDevice->getVKDevice(), _buffer, &memoryRequirements);

						// allocate memory for the buffer on the physical device
						VkMemoryAllocateInfo	memoryAllocateInfo{};
						memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
						memoryAllocateInfo.allocationSize = memoryRequirements.size;
						memoryAllocateInfo.memoryTypeIndex = _ftDevice->findMemoryType(memoryRequirements.memoryTypeBits, memoryProperties);

						if (vkAllocateMemory(_ftDevice->getVKDevice(), &memoryAllocateInfo, nullptr, &_bufferMemory) != VK_SUCCESS) {
							throw std::runtime_error("failed to allocate memory for the vertex buffer!");
						}

						// bind memory with the buffer
						vkBindBufferMemory(_ftDevice->getVKDevice(), _buffer, _bufferMemory, 0);

				   }

ft::Buffer::~Buffer() {
	vkDestroyBuffer(_ftDevice->getVKDevice(), _buffer, nullptr);
	vkFreeMemory(_ftDevice->getVKDevice(), _bufferMemory, nullptr);
}

VkBuffer ft::Buffer::getVKBuffer() const { return _buffer;}

/*******************************BufferBuilder****************************************/

ft::BufferBuilder& ft::BufferBuilder::setSize(VkDeviceSize size) {
	_size = size;
	return *this;
}

ft::BufferBuilder& ft::BufferBuilder::setUsageFlags(VkBufferUsageFlags usageFlags) {
	_usageFlags = usageFlags;
	return *this;
}

ft::BufferBuilder& ft::BufferBuilder::setMemoryProperties(VkMemoryPropertyFlags memoryProperties) {
	_memoryProperties = memoryProperties;
	return *this;
}

std::unique_ptr<ft::Buffer> ft::BufferBuilder::build(std::shared_ptr<Device> &device) {
	_ftBuffer = std::make_unique<Buffer>(device, _size, _usageFlags, _memoryProperties);
	return std::move(_ftBuffer);
}
