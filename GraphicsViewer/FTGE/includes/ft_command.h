#ifndef FTGRAPHICS_FT_COMMAND_H
#define FTGRAPHICS_FT_COMMAND_H

#include "ft_headers.h"
#include "ft_device.h"

namespace ft {

	class CommandPool {
	public:

		using pointer = std::shared_ptr<CommandPool>;

		CommandPool(Device::pointer device);
		~CommandPool();

		[[nodiscard]] VkCommandPool getVKCommandPool() const;

	private:
		Device::pointer 							_ftDevice;
		VkCommandPool 								_commandPool;
	};


	class CommandBuffer {

	public:

		using pointer = std::shared_ptr<CommandBuffer>;

		CommandBuffer(Device::pointer device,
					  CommandPool::pointer &commandPool,
					  VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY);
		~CommandBuffer();


		[[nodiscard]] VkCommandBuffer 	getVKCommandBuffer() const;
		[[nodiscard]] const VkCommandBuffer* getVKCommandBufferPtr() const;
		void 				beginRecording(VkCommandBufferUsageFlags flags);
		void 				reset(VkCommandBufferResetFlags flags = 0);
		void 				end();
		void 				submit(VkQueue	vkQueue, VkSubmitInfo submitInfo = {}, VkFence fence = VK_NULL_HANDLE);

	private:

		Device::pointer 						_ftDevice;
		CommandPool::pointer 					_ftCommandPool;
		VkCommandBuffer 						_commandBuffer;
		VkCommandBufferLevel					_commandBufferLevel;

	};
}


#endif //FTGRAPHICS_FT_COMMAND_H
