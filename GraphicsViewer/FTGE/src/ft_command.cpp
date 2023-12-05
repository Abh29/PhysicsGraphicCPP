#include "../include.h"
#include "../includes/ft_command.h"


ft::CommandPool::CommandPool(std::shared_ptr<Device>& device): _ftDevice(device) {

	QueueFamilyIndices queueFamilyIndices = _ftDevice->getQueueFamilyIndices();
	VkCommandPoolCreateInfo poolCreateInfo{};
	poolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	poolCreateInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

	if (vkCreateCommandPool(_ftDevice->getVKDevice(), &poolCreateInfo, nullptr, &_commandPool) != VK_SUCCESS) {
		throw std::runtime_error("failed to create a command pool!");
	}
}

ft::CommandPool::~CommandPool() {
	vkDestroyCommandPool(_ftDevice->getVKDevice(), _commandPool, nullptr);
}

VkCommandPool ft::CommandPool::getVKCommandPool() const {return _commandPool;}

/****************************************Command Buffer***********************/


ft::CommandBuffer::CommandBuffer(std::shared_ptr<Device> &device, std::shared_ptr<CommandPool> &commandPool,
								 VkCommandBufferLevel level) :
_ftDevice(device), _ftCommandPool(commandPool), _commandBufferLevel(level){

	VkCommandBufferAllocateInfo commandBufferAllocateInfo{};
	commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	commandBufferAllocateInfo.commandPool = _ftCommandPool->getVKCommandPool();
	commandBufferAllocateInfo.level = _commandBufferLevel;
	commandBufferAllocateInfo.commandBufferCount = 1;

	if (vkAllocateCommandBuffers(_ftDevice->getVKDevice(), &commandBufferAllocateInfo, &_commandBuffer) != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate a command buffer!");
	}
}

ft::CommandBuffer::~CommandBuffer() {
	vkFreeCommandBuffers(_ftDevice->getVKDevice(), _ftCommandPool->getVKCommandPool(), 1, &_commandBuffer);
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

