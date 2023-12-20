#ifndef FTGRAPHICS_FT_RENDERER_H
#define FTGRAPHICS_FT_RENDERER_H

#include "ft_headers.h"
#include "ft_defines.h"

namespace ft {

	class Renderer {

	public:
		using pointer = std::shared_ptr<Renderer>;

		Renderer(Device::pointer device);
		~Renderer();

		CommandBuffer::pointer beginFrame();
		void endFrame();
		void beginRenderPass(CommandBuffer::pointer commandBuffer);
		void endRenderPass(CommandBuffer::pointer commandBuffer);


	private:

		void createGraphicsPipeline();
		void createFrameBuffers();
		void recordCommandBuffer(const std::shared_ptr<CommandBuffer> &commandBuffer, uint32_t imageIndex);
		void createSyncObjects();
		void recreateSwapChain();
		void cleanUpSwapChain();
		void createDescriptorSetLayout();
		void createUniformBuffers();
		void createDescriptorPool();
		void createDescriptorSets();
		void createTextureImage();
		void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels);
		void createTextureSampler();
		void createDepthResources();
		void generateMipmaps(VkImage image, VkFormat imageFormat, int32_t texWidth, int32_t texHeight, uint32_t mipLevels);
		void createColorResources();


		Device::pointer 						_ftDevice;
		SwapChain::pointer 						_ftSwapChain;
		std::shared_ptr<ImageBuilder>			_ftImageBuilder;
		std::shared_ptr<BufferBuilder>			_ftBufferBuilder;
		std::vector<CommandBuffer::pointer>		_ftCommandBuffers;
		std::vector<VkFramebuffer>				_swapChainFramebuffers;
		std::vector<VkSemaphore>				_imageAvailableSemaphores;
		std::vector<VkSemaphore>				_renderFinishedSemaphores;
		std::vector<VkFence>					_inFlightFences;
		uint32_t 								_currentFrame = 0;
		VkDescriptorSetLayout 					_descriptorSetLayout;
		VkDescriptorPool 						_descriptorPool;
		std::vector<VkDescriptorSet>			_descriptorSets;
		uint32_t 								_mipLevels;
		VkSampler 								_textureSampler;
	};

}

#endif //FTGRAPHICS_FT_RENDERER_H
