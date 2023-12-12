#include "../includes/ft_app.h"

ft::Application::Application() :
_ftEventListener(std::make_shared<ft::EventListener>()),
_ftWindow{std::make_shared<Window>(W_WIDTH, W_HEIGHT, "applicationWindow", nullptr, _ftEventListener)}
{
	_validationLayers = {
			"VK_LAYER_KHRONOS_validation",
			"VK_LAYER_LUNARG_monitor"
	};

	_deviceExtensions = {
			VK_KHR_SWAPCHAIN_EXTENSION_NAME,
	};

	_ftEventListener->addCallbackForEventType(Event::EventType::KEYBOARD_EVENT, [&](ft::Event& ev) {
		ft::KeyboardEvent& kev = dynamic_cast<KeyboardEvent&>(ev);
		auto data = kev.getData();
		if (std::any_cast<int>(data[2]) == _ftWindow->ACTION(KeyActions::KEY_PRESS) ||
			std::any_cast<int>(data[2]) == _ftWindow->ACTION(KeyActions::KEY_REPEAT))
			updatePushConstant(std::any_cast<int>(data[0]));
	});

	initApplication();
	initPushConstants();
}

ft::Application::~Application() {
	cleanup();
}

void ft::Application::run() {

	while(!_ftWindow->shouldClose()) {
		_ftWindow->pollEvents();
		_ftGui->newFrame();
		_ftGui->showDemo();
		drawFrame();
		#ifdef SHOW_FRAME_RATE
			printFPS();
		#endif
	}
	vkDeviceWaitIdle(_ftDevice->getVKDevice());
}

void ft::Application::initApplication() {

	VkApplicationInfo	applicationInfo{};
	applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	applicationInfo.pApplicationName = "Simple Application";
	applicationInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	applicationInfo.pEngineName = "No Engine";
	applicationInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	applicationInfo.apiVersion = VK_MAKE_API_VERSION(0, 1, 3, 0);
	applicationInfo.pNext = nullptr;

	_ftInstance = std::make_shared<Instance>(applicationInfo, _validationLayers, _ftWindow->getRequiredExtensions());
	_ftSurface = std::make_shared<Surface>(_ftInstance, _ftWindow);
	_ftPhysicalDevice = std::make_shared<PhysicalDevice>(_ftInstance, _ftSurface, _deviceExtensions);
	_ftDevice = std::make_shared<Device>(_ftPhysicalDevice, _validationLayers, _deviceExtensions);
	_ftSwapChain = std::make_shared<SwapChain>(_ftPhysicalDevice, _ftDevice, _ftSurface,
											   _ftWindow->queryCurrentWidthHeight().first,
											   _ftWindow->queryCurrentWidthHeight().second,
											   VK_PRESENT_MODE_IMMEDIATE_KHR);
	_ftCommandPool = std::make_shared<CommandPool>(_ftDevice);
	for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
		_ftCommandBuffers.push_back(std::make_shared<CommandBuffer>(
				_ftDevice, _ftCommandPool));
	}

	_ftImageBuilder = std::make_shared<ImageBuilder>();
	_ftBufferBuilder = std::make_shared<BufferBuilder>();

	createColorResources();
	createDepthResources();
	initRenderPass();

	_ftGui = std::make_shared<Gui>(_ftInstance, _ftPhysicalDevice, _ftDevice,
								   _ftWindow, _ftRenderPass, _ftCommandPool,
								   MAX_FRAMES_IN_FLIGHT);

	createTextureImage();
	createDescriptorSetLayout();
	createGraphicsPipeline();
	createFramebuffers();
	createTextureSampler();
	loadModel();
	createVertexBuffer();
	createIndexBuffer();
	createPerInstanceBuffer();
	createUniformBuffers();
	createDescriptorPool();
	createDescriptorSets();
	createSyncObjects();

}

void ft::Application::initRenderPass() {
	ft::AttachmentBuilder attachmentBuilder;

	ft::Attachment::pointer colorAttachment = attachmentBuilder
			.setDescriptionFormat(_ftSwapChain->getVKSwapChainImageFormat())
			.setDescriptionSamples(_ftDevice->getMSAASamples())
			.setDescriptionLoadOp(VK_ATTACHMENT_LOAD_OP_CLEAR)
			.setDescriptionStoreOp(VK_ATTACHMENT_STORE_OP_STORE)
			.setDescriptionFinalLayout(VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
			.setReferenceImageLayout(VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
			.build(_ftColorImage);

	ft::Attachment::pointer depthAttachment = attachmentBuilder
			.setDescriptionFormat(_ftDevice->findDepthFormat())
			.setDescriptionStoreOp(VK_ATTACHMENT_STORE_OP_DONT_CARE)
			.setDescriptionFinalLayout(VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
			.setReferenceImageLayout(VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
			.build(_ftDepthImage);

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

void ft::Application::printFPS() {
	static auto oldTime = std::chrono::high_resolution_clock::now();
	static int fps;

	fps++;
	if (std::chrono::duration_cast<std::chrono::seconds>(std::chrono::high_resolution_clock::now() - oldTime) >= std::chrono::seconds{ 1 }) {
		oldTime = std::chrono::high_resolution_clock::now();
		std::cout << "FPS: " << fps <<  std::endl;
		fps = 0;
	}
}

void ft::Application::cleanup() {
	cleanUpSwapChain();
	vkDestroyPipeline(_ftDevice->getVKDevice(), _graphicsPipeline, nullptr);
	vkDestroyPipelineLayout(_ftDevice->getVKDevice(), _pipelineLayout, nullptr);

	vkDestroyDescriptorPool(_ftDevice->getVKDevice(), _descriptorPool, nullptr);
	vkDestroyDescriptorSetLayout(_ftDevice->getVKDevice(), _descriptorSetLayout, nullptr);

	vkDestroySampler(_ftDevice->getVKDevice(), _textureSampler, nullptr);

	for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
		vkDestroySemaphore(_ftDevice->getVKDevice(), _renderFinishedSemaphores[i], nullptr);
		vkDestroySemaphore(_ftDevice->getVKDevice(), _imageAvailableSemaphores[i], nullptr);
		vkDestroyFence(_ftDevice->getVKDevice(), _inFlightFences[i], nullptr);
	}

}

// graphics pipeline
void ft::Application::createGraphicsPipeline() {
	// create shader modules
	auto vertShaderCode = readFile("shaders/shader.vert.spv");
	auto fragShaderCode = readFile("shaders/shader.frag.spv");

	VkShaderModule vertShaderModule = createShaderModule(vertShaderCode);
	VkShaderModule fragShaderModule = createShaderModule(fragShaderCode);

	// vertex shader stage
	VkPipelineShaderStageCreateInfo vertShaderStageCreateInfo{};
	vertShaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertShaderStageCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vertShaderStageCreateInfo.module = vertShaderModule;
	vertShaderStageCreateInfo.pName = "main";
	vertShaderStageCreateInfo.pSpecializationInfo = nullptr; // for specifying constants

	// fragment shader stage
	VkPipelineShaderStageCreateInfo fragShaderStageCreationInfo{};
	fragShaderStageCreationInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragShaderStageCreationInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragShaderStageCreationInfo.module = fragShaderModule;
	fragShaderStageCreationInfo.pName = "main";
	fragShaderStageCreationInfo.pSpecializationInfo = nullptr;


	VkPipelineShaderStageCreateInfo shaderStages[] = {
			vertShaderStageCreateInfo,
			fragShaderStageCreationInfo
	};

	// dynamic states
	std::vector<VkDynamicState> dynamicStates = {
			VK_DYNAMIC_STATE_VIEWPORT,
			VK_DYNAMIC_STATE_SCISSOR,
//			VK_DYNAMIC_STATE_PRIMITIVE_TOPOLOGY
	};

	VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo{};
	dynamicStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicStateCreateInfo.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
	dynamicStateCreateInfo.pDynamicStates = dynamicStates.data();

	// vertex input state
	auto bindingDescription = Vertex::getBindingDescription();
	auto attributeDescriptions = Vertex::getAttributeDescription();

	auto perIndexBindingDescription = ft::InstanceDataType::getBindingDescription();
	auto perIndexattribsDescription = ft::InstanceDataType::getAttributeDescription();

	std::array<VkVertexInputBindingDescription, 2> bindings {
		bindingDescription,
		perIndexBindingDescription
	};

	std::vector<VkVertexInputAttributeDescription> attribs {attributeDescriptions.begin(), attributeDescriptions.end()};
	for(const auto& a : perIndexattribsDescription)
		attribs.push_back(a);

	VkPipelineVertexInputStateCreateInfo vertexInputStateCreateInfo{};
	vertexInputStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputStateCreateInfo.vertexBindingDescriptionCount = 2;
	vertexInputStateCreateInfo.pVertexBindingDescriptions = bindings.data();
	vertexInputStateCreateInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attribs.size());
	vertexInputStateCreateInfo.pVertexAttributeDescriptions = attribs.data();

	// depth and stencil state
	VkPipelineDepthStencilStateCreateInfo depthStencilStateCreateInfo{};
	depthStencilStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depthStencilStateCreateInfo.depthTestEnable = VK_TRUE;
	depthStencilStateCreateInfo.depthWriteEnable = VK_TRUE;
	depthStencilStateCreateInfo.depthCompareOp = VK_COMPARE_OP_LESS;
	depthStencilStateCreateInfo.depthBoundsTestEnable = VK_FALSE;
	depthStencilStateCreateInfo.minDepthBounds = 0.0f;
	depthStencilStateCreateInfo.maxDepthBounds = 1.0f;
	depthStencilStateCreateInfo.stencilTestEnable = VK_FALSE;
	depthStencilStateCreateInfo.front = {};
	depthStencilStateCreateInfo.back = {};

	// input assembly
	VkPipelineInputAssemblyStateCreateInfo inputAssemblyStateCreateInfo{};
	inputAssemblyStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssemblyStateCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
//		inputAssemblyStateCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
//		inputAssemblyStateCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
	inputAssemblyStateCreateInfo.primitiveRestartEnable = VK_FALSE;

	// view port and scissors
	VkViewport viewport{};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = (float) _ftSwapChain->getVKSwapChainExtent().width;
	viewport.height = (float) _ftSwapChain->getVKSwapChainExtent().height;

	VkRect2D scissor{};
	scissor.offset = {0, 0};
	scissor.extent = _ftSwapChain->getVKSwapChainExtent();

	VkPipelineViewportStateCreateInfo viewportStateCreateInfo{};
	viewportStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportStateCreateInfo.viewportCount = 1;
	viewportStateCreateInfo.pViewports = &viewport;
	viewportStateCreateInfo.scissorCount = 1;
	viewportStateCreateInfo.pScissors = &scissor;


	// rasterizer
	VkPipelineRasterizationStateCreateInfo rasterizerCreateInfo{};
	rasterizerCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizerCreateInfo.depthBiasClamp = VK_FALSE;
	rasterizerCreateInfo.rasterizerDiscardEnable = VK_FALSE;
	rasterizerCreateInfo.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizerCreateInfo.lineWidth = 1.0f;
	rasterizerCreateInfo.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizerCreateInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rasterizerCreateInfo.depthBiasEnable = VK_FALSE;
	rasterizerCreateInfo.depthBiasConstantFactor = 0.0f;
	rasterizerCreateInfo.depthBiasClamp = 0.0f;
	rasterizerCreateInfo.depthBiasSlopeFactor = 0.0f;

	// multisampling
	VkPipelineMultisampleStateCreateInfo multisampleStateCreateInfo{};
	multisampleStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	//		multisampleStateCreateInfo.sampleShadingEnable = VK_TRUE;
	//		multisampleStateCreateInfo.minSampleShading = 0.2f;
	multisampleStateCreateInfo.sampleShadingEnable = VK_FALSE;
	multisampleStateCreateInfo.rasterizationSamples = _ftDevice->getMSAASamples();
	multisampleStateCreateInfo.pSampleMask = nullptr;
	multisampleStateCreateInfo.alphaToCoverageEnable = VK_FALSE;
	multisampleStateCreateInfo.alphaToOneEnable = VK_FALSE;

	// color blending
	VkPipelineColorBlendAttachmentState colorBlendAttachmentState{};
	colorBlendAttachmentState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT
											   | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachmentState.blendEnable = VK_FALSE;
	colorBlendAttachmentState.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
	colorBlendAttachmentState.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
	colorBlendAttachmentState.colorBlendOp = VK_BLEND_OP_ADD;
	colorBlendAttachmentState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	colorBlendAttachmentState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	colorBlendAttachmentState.alphaBlendOp = VK_BLEND_OP_ADD;

	VkPipelineColorBlendStateCreateInfo colorBlendStateCreateInfo{};
	colorBlendStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlendStateCreateInfo.logicOpEnable = VK_FALSE;
	colorBlendStateCreateInfo.logicOp = VK_LOGIC_OP_COPY;
	colorBlendStateCreateInfo.attachmentCount = 1;
	colorBlendStateCreateInfo.pAttachments = &colorBlendAttachmentState;
	colorBlendStateCreateInfo.blendConstants[0] = 0.0f;
	colorBlendStateCreateInfo.blendConstants[1] = 0.0f;
	colorBlendStateCreateInfo.blendConstants[2] = 0.0f;
	colorBlendStateCreateInfo.blendConstants[3] = 0.0f;

	// push constants
	VkPushConstantRange pushConstantRange{};
	pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	pushConstantRange.size = static_cast<uint32_t>(sizeof(UniformBufferObject));
	pushConstantRange.offset = 0;

	// pipeline layout
	VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{};
	pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutCreateInfo.setLayoutCount = 1;
	pipelineLayoutCreateInfo.pSetLayouts = &_descriptorSetLayout;
	pipelineLayoutCreateInfo.pushConstantRangeCount = 1;
	pipelineLayoutCreateInfo.pPushConstantRanges = &pushConstantRange;

	if (vkCreatePipelineLayout(_ftDevice->getVKDevice(), &pipelineLayoutCreateInfo, nullptr, &_pipelineLayout) != VK_SUCCESS) {
		throw std::runtime_error("failed to create a pipeline layout!");
	}

	// creating the pipeline
	VkGraphicsPipelineCreateInfo pipelineCreateInfo{};
	pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineCreateInfo.stageCount = 2;
	pipelineCreateInfo.pStages = shaderStages;
	pipelineCreateInfo.pVertexInputState = &vertexInputStateCreateInfo;
	pipelineCreateInfo.pInputAssemblyState = &inputAssemblyStateCreateInfo;
	pipelineCreateInfo.pViewportState = &viewportStateCreateInfo;
	pipelineCreateInfo.pRasterizationState = &rasterizerCreateInfo;
	pipelineCreateInfo.pMultisampleState = &multisampleStateCreateInfo;
	pipelineCreateInfo.pDepthStencilState = &depthStencilStateCreateInfo;
	pipelineCreateInfo.pColorBlendState = &colorBlendStateCreateInfo;
	pipelineCreateInfo.pDynamicState = &dynamicStateCreateInfo;
	pipelineCreateInfo.layout = _pipelineLayout;
	pipelineCreateInfo.renderPass = _ftRenderPass->getVKRenderPass();
	pipelineCreateInfo.subpass = 0; // index of sub pass
	pipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;
	pipelineCreateInfo.basePipelineIndex = -1;

	if (vkCreateGraphicsPipelines(_ftDevice->getVKDevice(), VK_NULL_HANDLE, 1, &pipelineCreateInfo, nullptr, &_graphicsPipeline) != VK_SUCCESS) {
		throw std::runtime_error("failed to create a graphics pipeline!");
	}

	vkDestroyShaderModule(_ftDevice->getVKDevice(), vertShaderModule, nullptr);
	vkDestroyShaderModule(_ftDevice->getVKDevice(), fragShaderModule, nullptr);
}

// render module
VkShaderModule ft::Application::createShaderModule(const std::vector<char>& code) {
	VkShaderModuleCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = code.size();
	createInfo.pCode = reinterpret_cast<const uint32_t *>(code.data());
	// the alignment requirements are satisfied by std::allocator in std::vector
	VkShaderModule shaderModule;
	if (vkCreateShaderModule(_ftDevice->getVKDevice(), &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
		throw std::runtime_error("failed to create a shader module!");
	}
	return shaderModule;
}

// framebuffers
void ft::Application::createFramebuffers() {
	_swapChainFramebuffers.resize(_ftSwapChain->getVKSwapChainImageViews().size());
	for (size_t i = 0; i < _ftSwapChain->getVKSwapChainImageViews().size(); ++i) {
		std::array<VkImageView, 3> attachments = {
				_ftColorImage->getVKImageView(),
				_ftDepthImage->getVKImageView(),
				_ftSwapChain->getVKSwapChainImageViews()[i]
		};

		VkFramebufferCreateInfo framebufferCreateInfo{};
		framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferCreateInfo.renderPass = _ftRenderPass->getVKRenderPass();
		framebufferCreateInfo.attachmentCount = (uint32_t) attachments.size() ;
		framebufferCreateInfo.pAttachments = attachments.data();
		framebufferCreateInfo.width = _ftSwapChain->getVKSwapChainExtent().width;
		framebufferCreateInfo.height = _ftSwapChain->getVKSwapChainExtent().height;
		framebufferCreateInfo.layers = 1;

		if (vkCreateFramebuffer(_ftDevice->getVKDevice(), &framebufferCreateInfo, nullptr, &_swapChainFramebuffers[i]) != VK_SUCCESS) {
			throw std::runtime_error("failed to create a framebuffer!");
		}
	}
}

// command buffer recording
void ft::Application::recordCommandBuffer(const std::shared_ptr<CommandBuffer> &commandBuffer, uint32_t imageIndex) {

	// begin command buffer
	commandBuffer->beginRecording(0);

	// starting render pass
	std::array<VkClearValue, 2> clearValues = {};
	clearValues[0].color =	{{0.0f, 0.0f, 0.0f, 1.0f}};
	clearValues[1].depthStencil = {1.0f, 0};


	VkRenderPassBeginInfo renderPassBeginInfo{};
	renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassBeginInfo.renderPass = _ftRenderPass->getVKRenderPass();
	renderPassBeginInfo.framebuffer = _swapChainFramebuffers[imageIndex];
	renderPassBeginInfo.renderArea.offset = {0, 0};
	renderPassBeginInfo.renderArea.extent = _ftSwapChain->getVKSwapChainExtent();
	renderPassBeginInfo.clearValueCount = (uint32_t) clearValues.size();
	renderPassBeginInfo.pClearValues = clearValues.data();

	vkCmdBeginRenderPass(commandBuffer->getVKCommandBuffer(), &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

	// bind the graphics pipeline
	vkCmdBindPipeline(commandBuffer->getVKCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, _graphicsPipeline);

	// bind the vertex buffer
	VkBuffer vertexBuffers[] = {_ftVertexBuffer->getVKBuffer()};
	VkDeviceSize offsets[] = {0};
	VkBuffer perInstanceBuffers[] = {_ftInstanceDataBuffer->getVKBuffer()};
	vkCmdBindVertexBuffers(commandBuffer->getVKCommandBuffer(), 0, 1, vertexBuffers, offsets);
	vkCmdBindVertexBuffers(commandBuffer->getVKCommandBuffer(), 3, 1, perInstanceBuffers, offsets);

	// bind the index buffer
	vkCmdBindIndexBuffer(commandBuffer->getVKCommandBuffer(), _ftIndexBuffer->getVKBuffer(), 0, VK_INDEX_TYPE_UINT32);

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

	// bind the descriptor sets
	vkCmdBindDescriptorSets(commandBuffer->getVKCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS,
							_pipelineLayout, 0, 1, &_descriptorSets[_currentFrame],
							0, nullptr);

	// push constants
	vkCmdPushConstants(commandBuffer->getVKCommandBuffer(), _pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT,
					   0, static_cast<uint32_t>(sizeof(_push)), &_push);


	// topology
//	switch (_topology) {
//		case 0:
//			vkCmdSetPrimitiveTopology(commandBuffer->getVKCommandBuffer(), VK_PRIMITIVE_TOPOLOGY_LINE_LIST);
//			break;
//		case 1:
//			vkCmdSetPrimitiveTopology(commandBuffer->getVKCommandBuffer(), VK_PRIMITIVE_TOPOLOGY_LINE_STRIP);
//			break;
//		case 2:
//			vkCmdSetPrimitiveTopology(commandBuffer->getVKCommandBuffer(), VK_PRIMITIVE_TOPOLOGY_LINE_LIST_WITH_ADJACENCY);
//			break;
//	}

	// issue the draw command
	// vkCmdDraw(commandBuffer, static_cast<uint32_t>(vertices.size()), 1, 0, 0);
	vkCmdDrawIndexed(commandBuffer->getVKCommandBuffer(), static_cast<uint32_t>(_indices.size()), 3, 0, 0, 0);


	// gui
	_ftGui->render(commandBuffer);

	// render pass end
	vkCmdEndRenderPass(commandBuffer->getVKCommandBuffer());

	// end command buffer
	commandBuffer->end();
}

// draw a frame
void ft::Application::drawFrame() {
	// wait for previous frame
	vkWaitForFences(_ftDevice->getVKDevice(), 1, &_inFlightFences[_currentFrame], VK_TRUE, UINT64_MAX);
	vkResetFences(_ftDevice->getVKDevice(), 1, &_inFlightFences[_currentFrame]);

	// acquire and image from the swap chain
	auto result = _ftSwapChain->acquireNextImage(_imageAvailableSemaphores[_currentFrame]);

	if (result.first == VK_ERROR_OUT_OF_DATE_KHR) {
		recreateSwapChain();
		return;
	}

	// update the uniform buffer object data
	updateUniformBuffer(_currentFrame);


	// recording the command buffer
//	_ftCommandBuffers[_currentFrame]->reset();
	recordCommandBuffer(_ftCommandBuffers[_currentFrame], result.second);



	// submitting the command buffer
	VkSemaphore waitSemaphores[] = {_imageAvailableSemaphores[_currentFrame]};
	VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
	VkSemaphore signalSemaphores[] = {_renderFinishedSemaphores[_currentFrame]};

	VkSubmitInfo submitInfo{};
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitSemaphores;
	submitInfo.pWaitDstStageMask = waitStages;
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphores;

	_ftCommandBuffers[_currentFrame]->submit(_ftDevice->getVKGraphicsQueue(), submitInfo, _inFlightFences[_currentFrame]);

	// presentation
	VkSwapchainKHR	swapChains[] = {_ftSwapChain->getVKSwapChain()};

	VkPresentInfoKHR presentInfoKhr{};
	presentInfoKhr.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfoKhr.waitSemaphoreCount = 1;
	presentInfoKhr.pWaitSemaphores = signalSemaphores;
	presentInfoKhr.swapchainCount = 1;
	presentInfoKhr.pSwapchains = swapChains;
	presentInfoKhr.pImageIndices = &result.second;
	presentInfoKhr.pResults = nullptr;

	VkResult result2 = vkQueuePresentKHR(_ftDevice->getVKPresentQueue(), &presentInfoKhr);

	if (result2 == VK_ERROR_OUT_OF_DATE_KHR || result2 == VK_SUBOPTIMAL_KHR ) {
		recreateSwapChain();
	} else if (result2 != VK_SUCCESS) {
		throw std::runtime_error("failed to acquire the swap chain image!");
	}

	_currentFrame = (_currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

// semaphores and fences
void ft::Application::createSyncObjects() {
	_imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
	_renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
	_inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

	VkSemaphoreCreateInfo semaphoreCreateInfo{};
	semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkFenceCreateInfo fenceCreateInfo{};
	fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT; // this is for the first call

	for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
		if (vkCreateSemaphore(_ftDevice->getVKDevice(), &semaphoreCreateInfo, nullptr, &_imageAvailableSemaphores[i]) != VK_SUCCESS ||
			vkCreateSemaphore(_ftDevice->getVKDevice(), &semaphoreCreateInfo, nullptr, &_renderFinishedSemaphores[i]) != VK_SUCCESS ||
			vkCreateFence(_ftDevice->getVKDevice(), &fenceCreateInfo, nullptr, &_inFlightFences[i]) != VK_SUCCESS) {
			throw std::runtime_error("failed to create sync objects!");
		}
	}

}

// recreate swap chain (resize window event)
void ft::Application::recreateSwapChain() {
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
	cleanUpSwapChain();

	_ftSwapChain = std::make_shared<SwapChain>(_ftPhysicalDevice, _ftDevice, _ftSurface,
											   _ftWindow->queryCurrentWidthHeight().first,
											   _ftWindow->queryCurrentWidthHeight().second,
											   preferredMode);

	createColorResources();
	createDepthResources();
	createFramebuffers();
}

void ft::Application::cleanUpSwapChain() {

	_ftColorImage.reset();

	for (auto fb : _swapChainFramebuffers) {
		vkDestroyFramebuffer(_ftDevice->getVKDevice(), fb, nullptr);
	}
	_ftSwapChain.reset();
}

// create and fill the input buffer
void ft::Application::createVertexBuffer() {
	// create a staging buffer with memory
	VkDeviceSize bufferSize = sizeof(_vertices[0]) * _vertices.size();
	auto stagingBuffer = _ftBufferBuilder->setSize(bufferSize)
			.setUsageFlags(VK_BUFFER_USAGE_TRANSFER_SRC_BIT)
			.setMemoryProperties(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)
			.setIsMapped(true)
			.build(_ftDevice);

	// fill the staging vertex buffer
	stagingBuffer->copyToMappedData(_vertices.data(), bufferSize);


	// create a dest buffer
	_ftVertexBuffer = _ftBufferBuilder->setSize(bufferSize)
			.setUsageFlags(VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT)
			.setMemoryProperties(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
			.build(_ftDevice);

	// copy the data
	stagingBuffer->copyToBuffer(_ftCommandPool, _ftVertexBuffer, bufferSize);
}

// create Index Buffer
void ft::Application::createIndexBuffer() {
	VkDeviceSize bufferSize = sizeof(_indices[0]) * _indices.size();

	// create a staging buffer
	auto stagingBuffer = _ftBufferBuilder->setSize(bufferSize)
			.setUsageFlags(VK_BUFFER_USAGE_TRANSFER_SRC_BIT)
			.setMemoryProperties(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)
			.setIsMapped(true)
			.build(_ftDevice);

	// write the data to the staging buffer
	stagingBuffer->copyToMappedData(_indices.data(), bufferSize);

	// create the index buffer
	_ftIndexBuffer = _ftBufferBuilder->setSize(bufferSize)
			.setUsageFlags(VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT)
			.setMemoryProperties(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
			.setIsMapped(false)
			.build(_ftDevice);

	// copy buffer
	stagingBuffer->copyToBuffer(_ftCommandPool, _ftIndexBuffer, bufferSize);
}

// create descriptor set layout
void ft::Application::createDescriptorSetLayout() {
	// create a descriptor binding for the uniform buffer object layout
	VkDescriptorSetLayoutBinding	uboLayoutBinding{};
	uboLayoutBinding.binding = 0;
	uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	uboLayoutBinding.descriptorCount = 1;
	uboLayoutBinding.stageFlags = VK_SHADER_STAGE_ALL_GRAPHICS;
	uboLayoutBinding.pImmutableSamplers = nullptr;

	// create a descriptor binding for the sampler layout
	VkDescriptorSetLayoutBinding	samplerLayoutBinding{};
	samplerLayoutBinding.binding = 1;
	samplerLayoutBinding.descriptorCount = 1;
	samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	std::vector<VkDescriptorSetLayoutBinding> bindings = {
			uboLayoutBinding,
			samplerLayoutBinding
	};


	VkDescriptorSetLayoutCreateInfo layoutCreateInfo{};
	layoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutCreateInfo.bindingCount = (uint32_t) bindings.size();
	layoutCreateInfo.pBindings = bindings.data();

	if (vkCreateDescriptorSetLayout(_ftDevice->getVKDevice(), &layoutCreateInfo, nullptr, &_descriptorSetLayout) != VK_SUCCESS) {
		throw std::runtime_error("failed to create a descriptor set layout!");
	}
}

// create uniform buffers
void ft::Application::createUniformBuffers() {
	VkDeviceSize bufferSize = sizeof(UniformBufferObject) * MAX_INSTANCE_COUNT;

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

void ft::Application::updateUniformBuffer(uint32_t currentImage) {
	static auto startTime = std::chrono::high_resolution_clock::now();

	auto currentTime = std::chrono::high_resolution_clock::now();
	float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();
	(void) time;
	// calculate the data
	UniformBufferObject ubo{};
//	ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(10.0f), glm::linearRand(glm::vec3(0.0f), glm::vec3(1.0f)));
	ubo.model = glm::rotate(glm::mat4(1.0f), time  * glm::radians(10.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	ubo.proj = glm::perspective(glm::radians(45.0f), (float)_ftSwapChain->getVKSwapChainExtent().width / (float)_ftSwapChain->getVKSwapChainExtent().height, 1.0f, 10.0f);
	ubo.proj[1][1] *= -1;

	UniformBufferObject ubo2{};
	ubo2.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(15.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	ubo2.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	ubo2.proj = glm::perspective(glm::radians(45.0f), (float)_ftSwapChain->getVKSwapChainExtent().width / (float)_ftSwapChain->getVKSwapChainExtent().height, 1.0f, 10.0f);
	ubo2.proj[1][1] *= -1;

	UniformBufferObject ubo3{};
	ubo3.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(25.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	ubo3.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	ubo3.proj = glm::perspective(glm::radians(45.0f), (float)_ftSwapChain->getVKSwapChainExtent().width / (float)_ftSwapChain->getVKSwapChainExtent().height, 1.0f, 10.0f);
	ubo3.proj[1][1] *= -1;

	std::vector<UniformBufferObject> v{ubo, ubo2, ubo3};
	// copy the data to the buffer
	_ftUniformBuffers[currentImage]->copyToMappedData(v.data(), sizeof(ubo) * v.size());
}

// create descriptor pool
void ft::Application::createDescriptorPool() {

	VkDescriptorPoolSize pool_sizes[] =
	{
			{ VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
			{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
			{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
			{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
	};

	VkDescriptorPoolCreateInfo pool_info = {};
	pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
	pool_info.maxSets = 1000;
	pool_info.poolSizeCount = std::size(pool_sizes);
	pool_info.pPoolSizes = pool_sizes;

//	std::array<VkDescriptorPoolSize, 2> poolSizes{};
//
//	// size of the ubo descriptor
//	poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
//	poolSizes[0].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
//
//	// size of the sampler descriptor
//	poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
//	poolSizes[1].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT) * 2;
//
//	VkDescriptorPoolCreateInfo descriptorPoolCreateInfo{};
//	descriptorPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
//	descriptorPoolCreateInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
//	descriptorPoolCreateInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
//	descriptorPoolCreateInfo.pPoolSizes = poolSizes.data();
//	descriptorPoolCreateInfo.maxSets = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT) * 2;

	if (vkCreateDescriptorPool(_ftDevice->getVKDevice(), &pool_info, nullptr, &_descriptorPool) != VK_SUCCESS) {
		throw std::runtime_error("failed to create a descriptor pool!");
	}
}

// create Descriptor sets
void ft::Application::createDescriptorSets() {
	_descriptorSets.resize(MAX_FRAMES_IN_FLIGHT);

	// allocate the descriptor sets
	std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, _descriptorSetLayout);
	VkDescriptorSetAllocateInfo descriptorSetAllocateInfo{};
	descriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	descriptorSetAllocateInfo.descriptorPool = _descriptorPool;
	descriptorSetAllocateInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
	descriptorSetAllocateInfo.pSetLayouts = layouts.data();

	if (vkAllocateDescriptorSets(_ftDevice->getVKDevice(), &descriptorSetAllocateInfo, _descriptorSets.data()) != VK_SUCCESS) {
		throw std::runtime_error("failed to create the descriptor sets!");
	}

	// populate every descriptor
	for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
		// this is for ubo descriptor
		VkDescriptorBufferInfo descriptorBufferInfo{};
		descriptorBufferInfo.buffer = _ftUniformBuffers[i]->getVKBuffer();
		descriptorBufferInfo.offset = 0;
		descriptorBufferInfo.range = sizeof(UniformBufferObject) * MAX_INSTANCE_COUNT;

		std::array<VkWriteDescriptorSet, 2> writeDescriptorSets{};
		writeDescriptorSets[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		writeDescriptorSets[0].dstSet = _descriptorSets[i];
		writeDescriptorSets[0].dstBinding = 0;
		writeDescriptorSets[0].dstArrayElement = 0;
		writeDescriptorSets[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		writeDescriptorSets[0].descriptorCount = 1;
		writeDescriptorSets[0].pBufferInfo = &descriptorBufferInfo;
		writeDescriptorSets[0].pImageInfo = nullptr;
		writeDescriptorSets[0].pTexelBufferView = nullptr;

		// this is for sampler descriptor
		VkDescriptorImageInfo descriptorImageInfo{};
		descriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		descriptorImageInfo.imageView = _ftTextureImage->getVKImageView();
		descriptorImageInfo.sampler = _textureSampler;

		writeDescriptorSets[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		writeDescriptorSets[1].dstSet = _descriptorSets[i];
		writeDescriptorSets[1].dstBinding = 1;
		writeDescriptorSets[1].dstArrayElement = 0;
		writeDescriptorSets[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		writeDescriptorSets[1].descriptorCount = 1;
		writeDescriptorSets[1].pBufferInfo = nullptr;
		writeDescriptorSets[1].pImageInfo = &descriptorImageInfo;
		writeDescriptorSets[1].pTexelBufferView = nullptr;

		vkUpdateDescriptorSets(_ftDevice->getVKDevice(), (uint32_t) writeDescriptorSets.size(), writeDescriptorSets.data(), 0, nullptr);
	}
}

// load images and create texture
void ft::Application::createTextureImage() {
	// load the image file
	int texWidth, texHeight, texChannels;
	stbi_uc *pixels = stbi_load(TEXTURE_PATH.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
	VkDeviceSize imageSize = texWidth * texHeight * 4;

	_mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(texWidth, texHeight)))) + 1;

	if (!pixels) {
		throw std::runtime_error("failed to load texture image!");
	}

	// staging buffer and memory
	auto stagingBuffer = _ftBufferBuilder->setSize(imageSize)
			.setUsageFlags(VK_BUFFER_USAGE_TRANSFER_SRC_BIT)
			.setMemoryProperties(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)
			.setIsMapped(true)
			.build(_ftDevice);

	stagingBuffer->copyToMappedData(pixels, imageSize);

	// create texture image on device with memory
	_ftTextureImage = _ftImageBuilder->setWidthHeight(texWidth, texHeight)
			.setMipLevel(_mipLevels)
			.setSampleCount(VK_SAMPLE_COUNT_1_BIT)
			.setFormat(VK_FORMAT_R8G8B8A8_SRGB)
			.setTiling(VK_IMAGE_TILING_OPTIMAL)
			.setUsageFlags(VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT)
			.setMemoryProperties(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
			.setAspectFlags(VK_IMAGE_ASPECT_COLOR_BIT)
			.build(_ftDevice);

	// transition the image layout for dst copy
	transitionImageLayout(_ftTextureImage->getVKImage(), VK_FORMAT_R8G8B8A8_SRGB,
						  VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, _mipLevels);

	// copy the buffer to the image
	stagingBuffer->copyToImage(_ftCommandPool, _ftTextureImage, texWidth, texHeight);

	// generate mip maps
	generateMipmaps(_ftTextureImage->getVKImage(), VK_FORMAT_R8G8B8A8_SRGB, texWidth, texHeight, _mipLevels);

	// clean up
	stbi_image_free(pixels);
}

void ft::Application::transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels) {

	// create a command buffer for transition
	std::unique_ptr<CommandBuffer>	commandBuffer = std::make_unique<CommandBuffer>(_ftDevice, _ftCommandPool);
	commandBuffer->beginRecording(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);


	// create a barrier for sync
	VkImageMemoryBarrier barrier{};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.oldLayout = oldLayout;
	barrier.newLayout = newLayout;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.image = image;
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = mipLevels;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;

	// transition barrier masks
	VkPipelineStageFlags sourceStage;
	VkPipelineStageFlags destinationStage;

	if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	} else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	} else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
		// set the right aspectMask
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
		barrier.subresourceRange.aspectMask |= _ftDevice->hasStencilComponent(format) ? VK_IMAGE_ASPECT_STENCIL_BIT : VK_IMAGE_ASPECT_NONE;
		barrier.srcAccessMask = VK_ACCESS_NONE;
		barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
								VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	}else {
		throw std::invalid_argument("unsopported layout transition!");
	}

	// submit the barrier
	vkCmdPipelineBarrier(commandBuffer->getVKCommandBuffer(),
						 sourceStage, destinationStage,
						 0, 0, nullptr, 0, nullptr,
						 1,&barrier);

	// execution and clean up
	commandBuffer->end();
	commandBuffer->submit(_ftDevice->getVKGraphicsQueue());
}

// texture image sampler
void ft::Application::createTextureSampler() {

	VkPhysicalDeviceProperties properties = _ftDevice->getVKPhysicalDeviceProperties();

	VkSamplerCreateInfo samplerCreateInfo{};
	samplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerCreateInfo.magFilter = VK_FILTER_LINEAR;
	samplerCreateInfo.minFilter = VK_FILTER_LINEAR;
	samplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerCreateInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerCreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerCreateInfo.anisotropyEnable = VK_TRUE;
	samplerCreateInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
	samplerCreateInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	samplerCreateInfo.unnormalizedCoordinates = VK_FALSE;
	samplerCreateInfo.compareEnable = VK_FALSE;
	samplerCreateInfo.compareOp = VK_COMPARE_OP_ALWAYS;
	samplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerCreateInfo.mipLodBias = 0.0f;
	samplerCreateInfo.minLod = 0.0f;
	samplerCreateInfo.maxLod = VK_LOD_CLAMP_NONE;

	if (vkCreateSampler(_ftDevice->getVKDevice(), &samplerCreateInfo, nullptr, &_textureSampler) != VK_SUCCESS) {
		throw std::runtime_error("failed to create a texture sampler!");
	}
}

// depth image and view
void ft::Application::createDepthResources() {
	// find optimal depth format supported by the device
	VkFormat depthFormat = _ftDevice->findDepthFormat();
	_ftDepthImage = _ftImageBuilder->setWidthHeight(_ftSwapChain->getWidth(), _ftSwapChain->getHeight())
			.setMipLevel(1)
			.setSampleCount(_ftDevice->getMSAASamples())
			.setFormat(depthFormat)
			.setTiling(VK_IMAGE_TILING_OPTIMAL)
			.setUsageFlags(VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)
			.setMemoryProperties(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
			.setAspectFlags(VK_IMAGE_ASPECT_DEPTH_BIT)
			.build(_ftDevice);

	transitionImageLayout(_ftDepthImage->getVKImage(), depthFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, 1);
}

// loading a model
void ft::Application::loadModel() {
	tinyobj::attrib_t	attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	std::string warn, err;

	if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, MODEL_PATH.c_str())) {
		throw std::runtime_error(warn + err);
	}

	std::unordered_map<Vertex, uint32_t> uniqueVertices{};

	for (const auto& shape: shapes) {
		for (const auto& index: shape.mesh.indices) {
			Vertex vertex{};

			vertex.pos = {
					attrib.vertices[3 * index.vertex_index + 0],
					attrib.vertices[3 * index.vertex_index + 1],
					attrib.vertices[3 * index.vertex_index + 2],
			};

			vertex.texCoord =  {
					attrib.texcoords[2 * index.texcoord_index + 0],
					1.0f - attrib.texcoords[2 * index.texcoord_index + 1],
			};

			vertex.color = {1.0f, 1.0f, 1.0f};

			if (uniqueVertices.count(vertex) == 0) {
				uniqueVertices[vertex] = static_cast<uint32_t>(_vertices.size());
				_vertices.push_back(vertex);
			}

			_indices.push_back(uniqueVertices[vertex]);
		}
	}

	std::cout << "vertices: " << _vertices.size() << std::endl;
	std::cout << "indices: " << _indices.size() << std::endl;

}

// generate mipmaps
void ft::Application::generateMipmaps(VkImage image, VkFormat imageFormat, int32_t texWidth, int32_t texHeight, uint32_t mipLevels) {
	// check if the image format supports linear blitting
	VkFormatProperties formatProperties = _ftDevice->getVKFormatProperties(imageFormat);

	if (!(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT)) {
		throw std::runtime_error("texture image format does not support linear blitting!");
	}

	std::unique_ptr<CommandBuffer>	commandBuffer = std::make_unique<CommandBuffer>(_ftDevice, _ftCommandPool);
	commandBuffer->beginRecording(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);


	VkImageMemoryBarrier barrier{};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.image = image;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.levelCount = 1;
	barrier.subresourceRange.layerCount = 1;


	int32_t mipWidth = texWidth;
	int32_t mipHeight = texHeight;

	for (uint32_t i = 1; i < mipLevels; ++i) {
		barrier.subresourceRange.baseMipLevel = i - 1;
		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

		vkCmdPipelineBarrier(commandBuffer->getVKCommandBuffer(), VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
							 0, nullptr, 0, nullptr, 1, &barrier);


		VkImageBlit imageBlit{};
		imageBlit.srcOffsets[0] = {0, 0, 0};
		imageBlit.srcOffsets[1] = {mipWidth, mipHeight, 1};
		imageBlit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		imageBlit.srcSubresource.mipLevel = i - 1;
		imageBlit.srcSubresource.baseArrayLayer = 0;
		imageBlit.srcSubresource.layerCount = 1;
		imageBlit.dstOffsets[0] = {0, 0, 0};
		imageBlit.dstOffsets[1] = {mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1};
		imageBlit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		imageBlit.dstSubresource.mipLevel = i;
		imageBlit.dstSubresource.baseArrayLayer = 0;
		imageBlit.dstSubresource.layerCount = 1;

		vkCmdBlitImage(commandBuffer->getVKCommandBuffer(), image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, image,
					   VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &imageBlit, VK_FILTER_LINEAR);


		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		vkCmdPipelineBarrier(commandBuffer->getVKCommandBuffer(), VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
							 0, nullptr, 0, nullptr, 1, &barrier);

		if (mipWidth > 1) mipWidth /= 2;
		if (mipHeight > 1) mipHeight /= 2;
	}


	barrier.subresourceRange.baseMipLevel = mipLevels - 1;
	barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

	vkCmdPipelineBarrier(commandBuffer->getVKCommandBuffer(), VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
						 0, nullptr, 0, nullptr, 1, &barrier);

	commandBuffer->end();
	commandBuffer->submit(_ftDevice->getVKGraphicsQueue());
}

// create multisampling image target
void ft::Application::createColorResources() {
	_ftColorImage = _ftImageBuilder->setWidthHeight(_ftSwapChain->getWidth(), _ftSwapChain->getHeight())
			.setMipLevel(1)
			.setSampleCount(_ftDevice->getMSAASamples())
			.setFormat(_ftSwapChain->getVKSwapChainImageFormat())
			.setTiling(VK_IMAGE_TILING_OPTIMAL)
			.setUsageFlags(VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT)
			.setMemoryProperties(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
			.setAspectFlags(VK_IMAGE_ASPECT_COLOR_BIT)
			.build(_ftDevice);
}

std::vector<char> ft::Application::readFile(const std::string& filename) {
	std::ifstream file(filename, std::ios::ate | std::ios::binary);
	if (!file.is_open()) {
		throw std::runtime_error("failed to open file " + filename);
	}
	std::streamsize fileSize = file.tellg();
	std::vector<char> buffer(fileSize);
	file.seekg(0);
	file.read(buffer.data(), fileSize);
	file.close();
	return buffer;
}

void ft::Application::updatePushConstant(int key) {
	std::cout << "key: " << key << std::endl;
	if (key == _ftWindow->KEY(KeyboardKeys::KEY_UP)) {
		_push.view = glm::rotate(_push.view, glm::radians(5.0f), {0.0f, 1.0f, 0.0f});
	} else if (key == _ftWindow->KEY(KeyboardKeys::KEY_DOWN)) {
		_push.view = glm::rotate(_push.view, glm::radians(5.0f), {0.0f, -1.0f, 0.0f});
	} else if (key == _ftWindow->KEY(KeyboardKeys::KEY_RIGHT)) {
		_push.view = glm::rotate(_push.view, glm::radians(5.0f), {1.0f, 0.0f, 0.0f});
	} else if (key == _ftWindow->KEY(KeyboardKeys::KEY_LEFT)) {
		_push.view = glm::rotate(_push.view, glm::radians(5.0f), {-1.0f, 0.0f, 0.0f});
	} else if (key == _ftWindow->KEY(KeyboardKeys::KEY_R)) {
		_push.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	} else if (key == _ftWindow->KEY(KeyboardKeys::KEY_W)) {
		_push.view = glm::lookAt(glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	} else if (key == _ftWindow->KEY(KeyboardKeys::KEY_A)) {
		_push.view = glm::lookAt(glm::vec3(3.0f, 3.0f, 3.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	} else if (key == _ftWindow->KEY(KeyboardKeys::KEY_S)) {
		_push.view = glm::lookAt(glm::vec3(4.0f, 4.0f, 4.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	} else if (key == _ftWindow->KEY(KeyboardKeys::KEY_D)) {
		_push.view = glm::lookAt(glm::vec3(5.0f, 5.0f, 5.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	} else if (key == _ftWindow->KEY(KeyboardKeys::KEY_Q)) {
		_push.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	} else if (key == _ftWindow->KEY(KeyboardKeys::KEY_E)) {
		_push.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	} else if (key == _ftWindow->KEY(KeyboardKeys::KEY_Z)) {
		_topology = (_topology + 1) % 3;
	}
}

void ft::Application::initPushConstants() {
	_push.model = glm::rotate(glm::mat4(1.0f), glm::radians(0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	_push.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	_push.proj = glm::perspective(glm::radians(45.0f), (float)_ftSwapChain->getVKSwapChainExtent().width / (float)_ftSwapChain->getVKSwapChainExtent().height, 1.0f, 10.0f);
	_push.proj[1][1] *= -1;
}

void ft::Application::createPerInstanceBuffer() {
	// data
	std::vector<ft::InstanceDataType> v(2);
	v[0].model = glm::rotate(glm::mat4(1.0f), glm::radians(0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	v[1].model = glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	v[1].model = glm::translate(v[1].model, glm::vec3{1.f, 1.f, 1.f});

	// create a staging buffer with memory
	VkDeviceSize bufferSize = sizeof(v[0]) * v.size();
	auto stagingBuffer = _ftBufferBuilder->setSize(bufferSize)
			.setUsageFlags(VK_BUFFER_USAGE_TRANSFER_SRC_BIT)
			.setMemoryProperties(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)
			.setIsMapped(true)
			.build(_ftDevice);

	// fill the staging vertex buffer
	stagingBuffer->copyToMappedData(v.data(), bufferSize);


	// create a dest buffer
	_ftInstanceDataBuffer = _ftBufferBuilder->setSize(bufferSize)
			.setUsageFlags(VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT)
			.setMemoryProperties(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
			.build(_ftDevice);

	// copy the data
	stagingBuffer->copyToBuffer(_ftCommandPool, _ftInstanceDataBuffer, bufferSize);
}
