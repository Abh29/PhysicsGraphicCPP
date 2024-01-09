#include "../includes/ft_buffer.h"


ft::Buffer::Buffer(std::shared_ptr<Device> &device, VkDeviceSize size, VkBufferUsageFlags usageFlags,
				   VkMemoryPropertyFlags memoryProperties, bool mapped, VkDeviceSize mappedOffset,
				   VkMemoryMapFlags mappedFlags) :
				   _ftDevice{device}, _size(size), _isMapped(mapped)
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

						if (_isMapped)
							vkMapMemory(_ftDevice->getVKDevice(), _bufferMemory, mappedOffset, _size, mappedFlags, &_mappedData);

				   }

ft::Buffer::~Buffer() {
	if (_mappedData)
		vkUnmapMemory(_ftDevice->getVKDevice(), _bufferMemory);
	vkDestroyBuffer(_ftDevice->getVKDevice(), _buffer, nullptr);
	vkFreeMemory(_ftDevice->getVKDevice(), _bufferMemory, nullptr);
}

VkBuffer ft::Buffer::getVKBuffer() const { return _buffer;}

void* ft::Buffer::getMappedData() const {
	return _mappedData;
}

VkDeviceSize ft::Buffer::getSize() const {return _size;}

void ft::Buffer::copyToMappedData(void *src, uint32_t size, uint32_t offset) {
	assert(_isMapped);
	std::memcpy((u_char *)_mappedData + offset, src, size);
}

void ft::Buffer::copyToBuffer(ft::Buffer::pointer &dst,
							  VkDeviceSize size, VkDeviceSize srcOffset, VkDeviceSize dstOffset) const {
	VkBufferCopy	copyRegion{};
	copyRegion.srcOffset = srcOffset;
	copyRegion.dstOffset = dstOffset;
	copyRegion.size = size;

	std::unique_ptr<CommandBuffer>	commandBuffer = std::make_unique<CommandBuffer>(_ftDevice);
	commandBuffer->beginRecording(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
	vkCmdCopyBuffer(commandBuffer->getVKCommandBuffer(), _buffer, dst->_buffer, 1, &copyRegion);
	commandBuffer->end();
	commandBuffer->submit(_ftDevice->getVKGraphicsQueue());
}

bool ft::Buffer::isMapped() const {
	return _isMapped;
}

void ft::Buffer::copyToImage(ft::Image::pointer &image, uint32_t width,
							 uint32_t height) {

	std::unique_ptr<CommandBuffer>	commandBuffer = std::make_unique<CommandBuffer>(_ftDevice);
	commandBuffer->beginRecording(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

	VkBufferImageCopy region{};
	region.bufferOffset = 0;
	region.bufferRowLength = 0;
	region.bufferImageHeight = 0;
	region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	region.imageSubresource.mipLevel = 0;
	region.imageSubresource.baseArrayLayer = 0;
	region.imageSubresource.layerCount = 1;
	region.imageOffset = {0, 0, 0};
	region.imageExtent = {width, height, 1};

	vkCmdCopyBufferToImage(commandBuffer->getVKCommandBuffer(), _buffer, image->getVKImage(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

	commandBuffer->end();
	commandBuffer->submit(_ftDevice->getVKGraphicsQueue());
}

void ft::Buffer::copyFromImage(const Image::pointer &image, VkImageLayout layout) {

    std::unique_ptr<CommandBuffer>	commandBuffer = std::make_unique<CommandBuffer>(_ftDevice);
    commandBuffer->beginRecording(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

    VkBufferImageCopy region{};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;
    region.imageOffset = {0, 0, 0};
    region.imageExtent = {image->getWidth(), image->getHeight(), 1};

    vkCmdCopyImageToBuffer(commandBuffer->getVKCommandBuffer(), image->getVKImage(), layout, _buffer, 1, &region);

    commandBuffer->end();
    commandBuffer->submit(_ftDevice->getVKGraphicsQueue());
}


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

ft::Buffer::pointer ft::BufferBuilder::build(std::shared_ptr<Device> &device) {
	_ftBuffer = std::make_unique<Buffer>(device, _size, _usageFlags, _memoryProperties,
										 _isMapped, _mappedOffset, _mappedFlags);
	_isMapped = false;
	return std::move(_ftBuffer);
}

ft::BufferBuilder& ft::BufferBuilder::setIsMapped(bool isMapped) {
	_isMapped = isMapped;
	return *this;
}

ft::BufferBuilder& ft::BufferBuilder::setMappedOffset(VkDeviceSize mappedOffset) {
	_mappedOffset = mappedOffset;
	return *this;
}

ft::BufferBuilder& ft::BufferBuilder::setMappedFlags(VkMemoryMapFlags mappedFlags) {
	_mappedFlags = mappedFlags;
	return *this;
}
