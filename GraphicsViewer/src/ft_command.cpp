#include "../includes/ft_command.h"


ft::CommandBuffer::CommandBuffer(std::shared_ptr<Device> device, VkCommandBufferLevel level) :
_ftDevice(std::move(device)), _commandBufferLevel(level){
	VkCommandBufferAllocateInfo commandBufferAllocateInfo{};
	commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	commandBufferAllocateInfo.commandPool = _ftDevice->getVKCommandPool();
	commandBufferAllocateInfo.level = _commandBufferLevel;
	commandBufferAllocateInfo.commandBufferCount = 1;

	if (vkAllocateCommandBuffers(_ftDevice->getVKDevice(), &commandBufferAllocateInfo, &_commandBuffer) != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate a command buffer!");
	}
}

ft::CommandBuffer::~CommandBuffer() {
	vkFreeCommandBuffers(_ftDevice->getVKDevice(), _ftDevice->getVKCommandPool(), 1, &_commandBuffer);
}

VkCommandBuffer ft::CommandBuffer::getVKCommandBuffer() const {
	return _commandBuffer;
}

void ft::CommandBuffer::beginRecording(VkCommandBufferUsageFlags flags) {
	VkCommandBufferBeginInfo commandBufferBeginInfo{};
	commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	commandBufferBeginInfo.flags = flags;

	vkBeginCommandBuffer(_commandBuffer, &commandBufferBeginInfo);
}

void ft::CommandBuffer::submit(VkQueue vkQueue,VkSubmitInfo submitInfo, VkFence	fence) {
	// submit the command buffer for execution and wait
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &_commandBuffer;

	vkQueueSubmit(vkQueue, 1, &submitInfo, fence);
	vkQueueWaitIdle(vkQueue);
}

void ft::CommandBuffer::reset(VkCommandBufferResetFlags flags) {
	vkResetCommandBuffer(_commandBuffer, flags);
}

const VkCommandBuffer *ft::CommandBuffer::getVKCommandBufferPtr() const {
	return &_commandBuffer;
}

void ft::CommandBuffer::end() {
	if (vkEndCommandBuffer(_commandBuffer) != VK_SUCCESS) {
		throw std::runtime_error("failed to record a command buffer!");
	}
}

