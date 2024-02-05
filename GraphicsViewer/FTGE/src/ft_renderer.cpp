#include <utility>

#include "../includes/ft_renderer.h"

ft::Renderer::Renderer(Window::pointer window, Surface::pointer surface, PhysicalDevice::pointer physicalDevice,
					   Device::pointer device) :
					   _ftWindow(std::move(window)),
					   _ftSurface(std::move(surface)),
					   _ftPhysicalDevice(std::move(physicalDevice)),
					   _ftDevice(std::move(device))
					   {
	initRenderer();
					   }

ft::Renderer::~Renderer() {}

void ft::Renderer::initRenderer() {
	_ftSwapChain = std::make_shared<SwapChain>(_ftPhysicalDevice, _ftDevice, _ftSurface,
											   _ftWindow->queryCurrentWidthHeight().first,
											   _ftWindow->queryCurrentWidthHeight().second,
											   VK_PRESENT_MODE_IMMEDIATE_KHR);

	for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
		_ftCommandBuffers.push_back(std::make_shared<CommandBuffer>(_ftDevice));
	}

	_ftImageBuilder = std::make_shared<ImageBuilder>();
	_ftBufferBuilder = std::make_shared<BufferBuilder>();

	initRenderPasses();
	_ftSwapChain->createFrameBuffers(_ftRenderPass);
	createUniformBuffers();
}

std::pair<uint32_t, ft::CommandBuffer::pointer> ft::Renderer::beginFrame() {
	// acquire and image from the swap chain
	auto result = _ftSwapChain->acquireNextImage();

	if (result.first == VK_ERROR_OUT_OF_DATE_KHR) {
		recreateSwapChain();
		return {-1, nullptr};
	}

	auto commandBuffer = getCurrentCommandBuffer();
	commandBuffer->beginRecording(0);

	return {result.second, commandBuffer};
}

void ft::Renderer::endFrame(CommandBuffer::pointer commandBuffer, uint32_t imageIndex) {
	// end command buffer
	commandBuffer->end();

	auto result2 = _ftSwapChain->submit(commandBuffer, imageIndex);

	if (result2 == VK_ERROR_OUT_OF_DATE_KHR || result2 == VK_SUBOPTIMAL_KHR ) {
		recreateSwapChain();
	} else if (result2 != VK_SUCCESS) {
		throw std::runtime_error("failed to acquire the swap chain image!");
	}

	_currentFrame = (_currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void ft::Renderer::beginRenderPass(CommandBuffer::pointer commandBuffer, uint32_t imageIndex) {
	// starting render pass
	std::array<VkClearValue, 2> clearValues = {};
	clearValues[0].color =	{{0.0f, 0.0f, 0.0f, 1.0f}};
	clearValues[1].depthStencil = {1.0f, 0};


	VkRenderPassBeginInfo renderPassBeginInfo{};
	renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassBeginInfo.renderPass = _ftRenderPass->getVKRenderPass();
	renderPassBeginInfo.framebuffer = _ftSwapChain->getFrameBuffers()[imageIndex];
	renderPassBeginInfo.renderArea.offset = {0, 0};
	renderPassBeginInfo.renderArea.extent = _ftSwapChain->getVKSwapChainExtent();
	renderPassBeginInfo.clearValueCount = (uint32_t) clearValues.size();
	renderPassBeginInfo.pClearValues = clearValues.data();

	vkCmdBeginRenderPass(commandBuffer->getVKCommandBuffer(), &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

	// set viewport and scissor
	VkViewport viewport{};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = static_cast<float>(_ftSwapChain->getVKSwapChainExtent().width);
	viewport.height = static_cast<float>(_ftSwapChain->getVKSwapChainExtent().height);
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	vkCmdSetViewport(commandBuffer->getVKCommandBuffer(), 0, 1, &viewport);

	VkRect2D scissor{};
	scissor.offset = {0, 0};
	scissor.extent = _ftSwapChain->getVKSwapChainExtent();
	vkCmdSetScissor(commandBuffer->getVKCommandBuffer(), 0, 1, &scissor);
}

void ft::Renderer::endRenderPass(CommandBuffer::pointer commandBuffer, uint32_t imageIndex) {
	(void) imageIndex;
	// render pass end
	vkCmdEndRenderPass(commandBuffer->getVKCommandBuffer());
}

ft::CommandBuffer::pointer ft::Renderer::getCurrentCommandBuffer() {
	return _ftCommandBuffers[_currentFrame];
}

void ft::Renderer::recreateSwapChain() {
	// handle minimization
	int width = 0, height = 0;
	std::tie(width, height) = _ftWindow->queryCurrentWidthHeight();
	while (width == 0 || height == 0) {
		std::tie(width, height) = _ftWindow->queryCurrentWidthHeight();
		_ftWindow->waitEvents();
	}

	// handle resize
	vkDeviceWaitIdle(_ftDevice->getVKDevice());
	auto preferredMode = _ftSwapChain->getPreferredPresentMode();

	_ftSwapChain.reset();
	_ftSwapChain = std::make_shared<SwapChain>(_ftPhysicalDevice, _ftDevice, _ftSurface,
											   _ftWindow->queryCurrentWidthHeight().first,
											   _ftWindow->queryCurrentWidthHeight().second,
											   preferredMode);
	_ftSwapChain->createFrameBuffers(_ftRenderPass);
}

void ft::Renderer::initRenderPasses() {
	ft::AttachmentBuilder attachmentBuilder;

	ft::Attachment::pointer colorAttachment = attachmentBuilder
			.setDescriptionFormat(_ftSwapChain->getVKSwapChainImageFormat())
			.setDescriptionSamples(_ftDevice->getMSAASamples())
			.setDescriptionLoadOp(VK_ATTACHMENT_LOAD_OP_CLEAR)
			.setDescriptionStoreOp(VK_ATTACHMENT_STORE_OP_STORE)
			.setDescriptionFinalLayout(VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
			.setReferenceImageLayout(VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
			.build();

	ft::Attachment::pointer depthAttachment = attachmentBuilder
			.setDescriptionFormat(_ftDevice->findDepthFormat())
			.setDescriptionStoreOp(VK_ATTACHMENT_STORE_OP_DONT_CARE)
			.setDescriptionFinalLayout(VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
			.setReferenceImageLayout(VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
			.build();

	ft::Attachment::pointer colorResolver = attachmentBuilder
			.setDescriptionFormat(_ftSwapChain->getVKSwapChainImageFormat())
			.setDescriptionSamples(VK_SAMPLE_COUNT_1_BIT)
			.setDescriptionLoadOp(VK_ATTACHMENT_LOAD_OP_DONT_CARE)
			.setDescriptionStoreOp(VK_ATTACHMENT_STORE_OP_STORE)
			.setDescriptionFinalLayout(VK_IMAGE_LAYOUT_PRESENT_SRC_KHR)
			.setReferenceImageLayout(VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
			.build();

	ft::SubPass::pointer subPass = std::make_shared<ft::SubPass>(VK_PIPELINE_BIND_POINT_GRAPHICS);
	ft::SubpassDependency::pointer subpassDependency = std::make_shared<ft::SubpassDependency>(VK_SUBPASS_EXTERNAL, 0);

	subpassDependency->setSrcStageMask(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT)
			.setSrcAccessMask(VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT)
			.setDstStageMask(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT)
			.setDstAccessMask(VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT);

	_ftRenderPass = std::make_shared<RenderPass>(_ftDevice);
	_ftRenderPass->addSubpass(subPass);
	_ftRenderPass->addSubpassDependency(subpassDependency);
	_ftRenderPass->addColorAttachmentToSubpass(colorAttachment);
	_ftRenderPass->setDepthStencilAttachmentToSubpass(depthAttachment);
	_ftRenderPass->addResolveAttachmentToSubpass(colorResolver);
	_ftRenderPass->create();
}

ft::SwapChain::pointer ft::Renderer::getSwapChain() const {return _ftSwapChain;}

ft::RenderPass::pointer ft::Renderer::getRenderPass() const {return _ftRenderPass;}

void ft::Renderer::createUniformBuffers() {
	VkDeviceSize bufferSize = sizeof(UniformBufferObject);

	for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
		_ftUniformBuffers.push_back(_ftBufferBuilder->setSize(bufferSize)
											.setUsageFlags(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT)
											.setMemoryProperties(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)
											.setIsMapped(true)
											.setMappedOffset(0)
											.setMappedFlags(0)
											.build(_ftDevice));
	}
}

std::vector<ft::Buffer::pointer> ft::Renderer::getUniformBuffers() const {return _ftUniformBuffers;}

