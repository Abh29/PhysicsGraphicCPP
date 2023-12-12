#ifndef FTGRAPHICS_FT_APP_H
#define FTGRAPHICS_FT_APP_H

#include "ft_headers.h"
#include "ft_window.h"
#include "ft_instance.h"
#include "ft_surface.h"
#include "ft_physicalDevice.h"
#include "ft_device.h"
#include "ft_image.h"
#include "ft_buffer.h"
#include "ft_command.h"
#include "ft_vertex.h"
#include "ft_renderPass.h"
#include "ft_attachment.h"
#include "ft_event.h"
#include "ft_gui.h"

namespace ft {

	struct InstanceDataType {
		alignas(16) glm::mat4 model;
		alignas(16) glm::vec3 color;


		static VkVertexInputBindingDescription getBindingDescription() {
			VkVertexInputBindingDescription bindingDescription{};
			bindingDescription.binding = 3;
			bindingDescription.stride = sizeof(InstanceDataType);
			bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_INSTANCE;

			return bindingDescription;
		}

		static std::array<VkVertexInputAttributeDescription, 5> getAttributeDescription() {
			std::array<VkVertexInputAttributeDescription, 5> attributeDescriptions{};
			attributeDescriptions[0].binding = 3;
			attributeDescriptions[0].location = 3;
			attributeDescriptions[0].format = VK_FORMAT_R32G32B32A32_SFLOAT;
			attributeDescriptions[0].offset = offsetof(InstanceDataType, model);

			attributeDescriptions[1].binding = 3;
			attributeDescriptions[1].location = 4;
			attributeDescriptions[1].format = VK_FORMAT_R32G32B32A32_SFLOAT;
			attributeDescriptions[1].offset = offsetof(InstanceDataType, model) + 16;

			attributeDescriptions[2].binding = 3;
			attributeDescriptions[2].location = 5;
			attributeDescriptions[2].format = VK_FORMAT_R32G32B32A32_SFLOAT;
			attributeDescriptions[2].offset = offsetof(InstanceDataType, model) + 32;

			attributeDescriptions[3].binding = 3;
			attributeDescriptions[3].location = 6;
			attributeDescriptions[3].format = VK_FORMAT_R32G32B32A32_SFLOAT;
			attributeDescriptions[3].offset = offsetof(InstanceDataType, model) + 48;

			attributeDescriptions[4].binding = 3;
			attributeDescriptions[4].location = 7;
			attributeDescriptions[4].format = VK_FORMAT_R32G32B32_SFLOAT;
			attributeDescriptions[4].offset = offsetof(InstanceDataType, color);

			return attributeDescriptions;
		}
	};

	class Application {
	public:
		static constexpr uint32_t W_WIDTH = 800;
		static constexpr uint32_t W_HEIGHT = 600;
		static constexpr int MAX_FRAMES_IN_FLIGHT = 2;
		const std::string MODEL_PATH = "models/viking_room.obj";
		const std::string TEXTURE_PATH = "textures/viking_room.png";
		const uint32_t MAX_INSTANCE_COUNT = 100;

		Application();
		~Application();

		void run();

	private:
		void initApplication();
		void initRenderPass();
		void initPushConstants();
		void createPerInstanceBuffer();

		EventListener::pointer				_ftEventListener;
		Window::pointer						_ftWindow;
		Instance::pointer					_ftInstance;
		Surface::pointer					_ftSurface;
		PhysicalDevice::pointer				_ftPhysicalDevice;
		Device::pointer						_ftDevice;
		SwapChain::pointer					_ftSwapChain;
		std::vector<const char*> 			_validationLayers;
		std::vector<const char*> 			_deviceExtensions;
		std::shared_ptr<ImageBuilder>		_ftImageBuilder;
		std::shared_ptr<BufferBuilder>		_ftBufferBuilder;
		CommandPool::pointer				_ftCommandPool;
		std::vector<CommandBuffer::pointer>	_ftCommandBuffers;
		Gui::pointer 						_ftGui;

		Image::pointer						_ftTextureImage;
		Image::pointer						_ftDepthImage;
		Image::pointer						_ftColorImage;
		std::vector<Buffer::pointer>		_ftUniformBuffers;
		Buffer::pointer						_ftVertexBuffer;
		Buffer::pointer						_ftIndexBuffer;
		Buffer::pointer						_ftInstanceDataBuffer;
		RenderPass::pointer					_ftRenderPass;
		UniformBufferObject					_push{};
		int									_topology = 0;


		/****************************triangle app ************************/
		void printFPS();
		void updatePushConstant(int key);
		static std::vector<char> readFile(const std::string& filename);
		void cleanup();
		void createGraphicsPipeline();
		VkShaderModule createShaderModule(const std::vector<char>& code);
		void createFramebuffers();
		void recordCommandBuffer(const std::shared_ptr<CommandBuffer> &commandBuffer, uint32_t imageIndex);
		void drawFrame();
		void createSyncObjects();
		void recreateSwapChain();
		void cleanUpSwapChain();
		void createVertexBuffer();
		void createIndexBuffer();
		void createDescriptorSetLayout();
		void createUniformBuffers();
		void updateUniformBuffer(uint32_t currentImage);
		void createDescriptorPool();
		void createDescriptorSets();
		void createTextureImage();
		void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels);
		void createTextureSampler();
		void createDepthResources();
		void loadModel();
		void generateMipmaps(VkImage image, VkFormat imageFormat, int32_t texWidth, int32_t texHeight, uint32_t mipLevels);
		void createColorResources();

		VkPipelineLayout 						_pipelineLayout;
		VkPipeline 								_graphicsPipeline;
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
		std::vector<Vertex>						_vertices;
		std::vector<uint32_t>					_indices;

	};


}

#endif //FTGRAPHICS_FT_APP_H
