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

	initEventListener();
	initApplication();
	initPushConstants();
	createScene();
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

void ft::Application::initEventListener() {
	_ftEventListener->addCallbackForEventType(Event::EventType::KEYBOARD_EVENT, [&](ft::Event& ev) {
		ft::KeyboardEvent& kev = dynamic_cast<KeyboardEvent&>(ev);
		auto data = kev.getData();
		if (std::any_cast<int>(data[2]) == _ftWindow->ACTION(KeyActions::KEY_PRESS) ||
			std::any_cast<int>(data[2]) == _ftWindow->ACTION(KeyActions::KEY_REPEAT))
			updateScene(std::any_cast<int>(data[0]));
	});

	_ftEventListener->addCallbackForEventType(Event::EventType::MOUSE_BUTTON, [&](ft::Event& ev) {
		ft::CursorEvent& cev = dynamic_cast<CursorEvent&>(ev);
		auto data = cev.getData();
		if (std::any_cast<int>(data[1]) == _ftWindow->ACTION(KeyActions::KEY_PRESS)) {
			double x = std::any_cast<double>(data[3]);
			double y = std::any_cast<double>(data[4]);
			std::cout << "mouse clicked: " << x << " , " << y << std::endl;
		}
	});

	_ftEventListener->addCallbackForEventType(Event::EventType::MOUSE_SCROLL, [&](ft::Event& ev) {
		ft::ScrollEvent& sev = dynamic_cast<ScrollEvent&>(ev);
		auto data = sev.getData();
		auto yOff = std::any_cast<double>(data[1]);

		_ftScene->getCamera()->forward(yOff / 10);
		_ftScene->updateCameraUBO();

	});
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
											   VK_PRESENT_MODE_FIFO_KHR);
	for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
		_ftCommandBuffers.push_back(std::make_shared<CommandBuffer>(_ftDevice));
	}

	_ftImageBuilder = std::make_shared<ImageBuilder>();
	_ftBufferBuilder = std::make_shared<BufferBuilder>();

	initRenderPass();
	_ftSwapChain->createFrameBuffers(_ftRenderPass);

	_ftGui = std::make_shared<Gui>(_ftInstance, _ftPhysicalDevice, _ftDevice,
								   _ftWindow, _ftRenderPass, MAX_FRAMES_IN_FLIGHT);
	_ftSampler = std::make_shared<ft::Sampler>(_ftDevice);

	createTextureImage();
	createDescriptorSetLayout();
	createGraphicsPipeline();
	createUniformBuffers();
	createDescriptorPool();
	createDescriptorSets();
	createSyncObjects();

}

void ft::Application::createScene() {
	CameraBuilder cameraBuilder;
	_ftScene = std::make_shared<Scene>(_ftDevice, _ftUniformBuffers);
	_ftScene->setCamera(cameraBuilder.setEyePosition({5,-1,0})
				.setTarget({1,-1,0})
				.setUpDirection({0,1,0})
				.setFOV(90)
				.setZNearFar(0.5f, 30.0f)
				.setAspect(_ftSwapChain->getAspect())
				.build());
	_ftScene->setGeneralLight({1.0f,1.0f,1.0f}, {10.0, -50.0, 10.0}, 0.2f);
	ft::InstanceData data;
	uint32_t  id;	(void) id;

//	data.model = glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(0.0f, 0.0f, 1.0f));
//	data.model = glm::translate(data.model, glm::vec3{1.0f, 0.0f, 1.0f});
//	data.color = {0.0f, 2.0f, 0.9f};
//	auto id = _ftScene->addObjectToTheScene("models/viking_room.obj", data);
//
//	data.model = glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
//	data.model = glm::translate(data.model, glm::vec3{0.0f, 0.0f, 3.0f});
//	data.color = {0.9f, 2.0f, 0.9f};
//	id = _ftScene->addObjectCopyToTheScene(id, data);

	data.model = glm::rotate(glm::mat4(1.0f), glm::radians(0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	data.model = glm::scale(data.model, {10.0f, 0.1f, 0.1f});
	data.color = {0.9f, 0.0f, 0.0f};
	data.normalMatrix = glm::mat4(1.0f);
	id = _ftScene->addObjectToTheScene("models/arrow.obj", data);

	data.model = glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	data.model = glm::scale(data.model, {10.0f, 0.1f, 0.1f});
	data.color = {0.0f, 0.0f, 0.9f};
	data.normalMatrix = glm::mat4(1.0f);
	id = _ftScene->addObjectCopyToTheScene(id, data);

	data.model = glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	data.model = glm::scale(data.model, {10.0f, 0.1f, 0.1f});
	data.color = {0.0f, 0.9f, 0.0f};
	data.normalMatrix = glm::mat4(1.0f);
	id = _ftScene->addObjectCopyToTheScene(id, data);

	// plane
	data.model = glm::mat4(1.0f);
	data.color = {0.95f, .95f, .95f};
	data.normalMatrix = glm::mat4(1.0f);
	data.model = glm::scale(data.model, {100, 100, 100});
	id = _ftScene->addObjectToTheScene("models/plane.mtl.obj", data);



	data.model = glm::mat4(1.0f);
	data.color = {0.95f, 0.2f, 0.0f};
	data.normalMatrix = glm::mat4(1.0f);
//	data.model = glm::scale(data.model, {10, 10, 10});
	data.model = glm::rotate(data.model, glm::radians(90.0f), {1,0,0});
	id = _ftScene->addObjectToTheScene("models/cube.mtl.obj", data);

	data.model = glm::rotate(glm::mat4(1.0f), glm::radians(0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	data.model = glm::translate(data.model, glm::vec3{1.0f, -2.0f, 0.0f});
	data.color = {0.5f, 0.95f, 0.2f};
	id = _ftScene->addObjectCopyToTheScene(id, data);

//
//	data.model = glm::rotate(glm::mat4(1.0f), glm::radians(0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
//	data.model = glm::translate(data.model, glm::vec3{0.0f, 3.0f, 0.0f});
//	data.color = {0.7f, 0.2f, 0.4f};
//	id = _ftScene->addObjectCopyToTheScene(id, data);
//
//	id = _ftScene->addObjectToTheScene("models/smooth_vase.obj", {
//			glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f)),
//			glm::mat4(1.0f),
//			{0.95f, 0.2f, 0.0f}
//	});

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
	_ftSwapChain.reset();
	vkDestroyDescriptorPool(_ftDevice->getVKDevice(), _descriptorPool, nullptr);
	vkDestroyDescriptorSetLayout(_ftDevice->getVKDevice(), _descriptorSetLayout, nullptr);

	for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
		vkDestroySemaphore(_ftDevice->getVKDevice(), _renderFinishedSemaphores[i], nullptr);
		vkDestroySemaphore(_ftDevice->getVKDevice(), _imageAvailableSemaphores[i], nullptr);
		vkDestroyFence(_ftDevice->getVKDevice(), _inFlightFences[i], nullptr);
	}

}

// graphics pipeline
void ft::Application::createGraphicsPipeline() {

	ft::PipelineConfig pipelineConfig{};

	// shader modules
	pipelineConfig.vertShaderPath = "shaders/shader.vert.spv";
	pipelineConfig.fragShaderPath = "shaders/shader.frag.spv";
	pipelineConfig.vertShaderEntryPoint = "main";
	pipelineConfig.vertShaderEntryPoint = "main";

	// dynamic states
	pipelineConfig.dynamicStates.push_back(VK_DYNAMIC_STATE_VIEWPORT);
	pipelineConfig.dynamicStates.push_back(VK_DYNAMIC_STATE_SCISSOR);

	// vertex input state
	pipelineConfig.bindings.push_back(ft::Vertex::getBindingDescription());
	pipelineConfig.bindings.push_back(ft::InstanceData::getBindingDescription());

	auto vertexAttributes = ft::Vertex::getAttributeDescription();
	auto instanceAttributes = ft::InstanceData::getAttributeDescription();

	pipelineConfig.attributes.insert(pipelineConfig.attributes.begin(),
									 instanceAttributes.begin(),
									 instanceAttributes.end());
	pipelineConfig.attributes.insert(pipelineConfig.attributes.begin(),
									 vertexAttributes.begin(),
									 vertexAttributes.end());

	// depth and stencil state
	pipelineConfig.depthStencilState.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	pipelineConfig.depthStencilState.depthTestEnable = VK_TRUE;
	pipelineConfig.depthStencilState.depthWriteEnable = VK_TRUE;
	pipelineConfig.depthStencilState.depthCompareOp = VK_COMPARE_OP_LESS;
	pipelineConfig.depthStencilState.depthBoundsTestEnable = VK_FALSE;
	pipelineConfig.depthStencilState.minDepthBounds = 0.0f;
	pipelineConfig.depthStencilState.maxDepthBounds = 1.0f;
	pipelineConfig.depthStencilState.stencilTestEnable = VK_FALSE;
	pipelineConfig.depthStencilState.front = {};
	pipelineConfig.depthStencilState.back = {};

	// input assembly
	pipelineConfig.inputAssemblyState.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	pipelineConfig.inputAssemblyState.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	pipelineConfig.inputAssemblyState.primitiveRestartEnable = VK_FALSE;
//		pipelineConfig.inputAssemblyState.topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
//		pipelineConfig.inputAssemblyState.topology = VK_PRIMITIVE_TOPOLOGY_POINT_LIST;

	// view port and scissors
	pipelineConfig.viewport.x = 0.0f;
	pipelineConfig.viewport.y = 0.0f;
	pipelineConfig.viewport.width = (float) _ftSwapChain->getVKSwapChainExtent().width;
	pipelineConfig.viewport.height = (float) _ftSwapChain->getVKSwapChainExtent().height;

	pipelineConfig.scissor.offset = {0, 0};
	pipelineConfig.scissor.extent = _ftSwapChain->getVKSwapChainExtent();

	// rasterizer
	pipelineConfig.rasterizerState.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	pipelineConfig.rasterizerState.depthBiasClamp = VK_FALSE;
	pipelineConfig.rasterizerState.rasterizerDiscardEnable = VK_FALSE;
	pipelineConfig.rasterizerState.polygonMode = VK_POLYGON_MODE_FILL;
	pipelineConfig.rasterizerState.lineWidth = 1.0f;
	pipelineConfig.rasterizerState.cullMode = VK_CULL_MODE_NONE; // mode_back_bit
	pipelineConfig.rasterizerState.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	pipelineConfig.rasterizerState.depthBiasEnable = VK_FALSE;
	pipelineConfig.rasterizerState.depthBiasConstantFactor = 0.0f;
	pipelineConfig.rasterizerState.depthBiasClamp = 0.0f;
	pipelineConfig.rasterizerState.depthBiasSlopeFactor = 0.0f;

	// multisampling
	pipelineConfig.multisampleState.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	//		pipelineConfig.multisampleState.sampleShadingEnable = VK_TRUE;
	//		pipelineConfig.multisampleState.minSampleShading = 0.2f;
	pipelineConfig.multisampleState.sampleShadingEnable = VK_FALSE;
	pipelineConfig.multisampleState.rasterizationSamples = _ftDevice->getMSAASamples();
	pipelineConfig.multisampleState.pSampleMask = nullptr;
	pipelineConfig.multisampleState.alphaToCoverageEnable = VK_FALSE;
	pipelineConfig.multisampleState.alphaToOneEnable = VK_FALSE;

	// color blending
	VkPipelineColorBlendAttachmentState colorBlendAttachmentState{};
	colorBlendAttachmentState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
											   VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachmentState.blendEnable = VK_FALSE;
	colorBlendAttachmentState.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
	colorBlendAttachmentState.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
	colorBlendAttachmentState.colorBlendOp = VK_BLEND_OP_ADD;
	colorBlendAttachmentState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	colorBlendAttachmentState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	colorBlendAttachmentState.alphaBlendOp = VK_BLEND_OP_ADD;

	pipelineConfig.colorBlendAttachments.push_back(colorBlendAttachmentState);

	pipelineConfig.colorBlendState.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	pipelineConfig.colorBlendState.logicOpEnable = VK_FALSE;
	pipelineConfig.colorBlendState.logicOp = VK_LOGIC_OP_COPY;
	pipelineConfig.colorBlendState.blendConstants[0] = 0.0f;
	pipelineConfig.colorBlendState.blendConstants[1] = 0.0f;
	pipelineConfig.colorBlendState.blendConstants[2] = 0.0f;
	pipelineConfig.colorBlendState.blendConstants[3] = 0.0f;

	// push constants
	VkPushConstantRange pushConstantRange{};
	pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	pushConstantRange.size = static_cast<uint32_t>(sizeof(ft::PushConstantObject));
	pushConstantRange.offset = 0;

	pipelineConfig.pushConstantRanges.push_back(pushConstantRange);

	_ftGraphicsPipeline = std::make_shared<ft::GraphicsPipeline>(_ftDevice, _ftRenderPass, _descriptorSetLayout, pipelineConfig);
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
	renderPassBeginInfo.framebuffer = _ftSwapChain->getFrameBuffers()[imageIndex];
	renderPassBeginInfo.renderArea.offset = {0, 0};
	renderPassBeginInfo.renderArea.extent = _ftSwapChain->getVKSwapChainExtent();
	renderPassBeginInfo.clearValueCount = (uint32_t) clearValues.size();
	renderPassBeginInfo.pClearValues = clearValues.data();

	vkCmdBeginRenderPass(commandBuffer->getVKCommandBuffer(), &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
	// bind the graphics pipeline
	vkCmdBindPipeline(commandBuffer->getVKCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, _ftGraphicsPipeline->getVKPipeline());

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
							_ftGraphicsPipeline->getVKPipelineLayout(), 0, 1, &_descriptorSets[_currentFrame],
							0, nullptr);

	// scene
	_ftScene->drawScene(commandBuffer, _ftGraphicsPipeline, _currentFrame);
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

	// recording the command buffer
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

	_ftSwapChain.reset();
	_ftSwapChain = std::make_shared<SwapChain>(_ftPhysicalDevice, _ftDevice, _ftSurface,
											   _ftWindow->queryCurrentWidthHeight().first,
											   _ftWindow->queryCurrentWidthHeight().second,
											   preferredMode);
	_ftSwapChain->createFrameBuffers(_ftRenderPass);
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
		descriptorBufferInfo.range = sizeof(UniformBufferObject);

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
		descriptorImageInfo.sampler = _ftSampler->getVKSampler();

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
	stagingBuffer->copyToImage(_ftTextureImage, texWidth, texHeight);

	// generate mip maps
	generateMipmaps(_ftTextureImage->getVKImage(), VK_FORMAT_R8G8B8A8_SRGB, texWidth, texHeight, _mipLevels);

	// clean up
	stbi_image_free(pixels);
}

void ft::Application::transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels) {

	// create a command buffer for transition
	std::unique_ptr<CommandBuffer>	commandBuffer = std::make_unique<CommandBuffer>(_ftDevice);
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

// generate mipmaps
void ft::Application::generateMipmaps(VkImage image, VkFormat imageFormat, int32_t texWidth, int32_t texHeight, uint32_t mipLevels) {
	// check if the image format supports linear blitting
	VkFormatProperties formatProperties = _ftDevice->getVKFormatProperties(imageFormat);

	if (!(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT)) {
		throw std::runtime_error("texture image format does not support linear blitting!");
	}

	std::unique_ptr<CommandBuffer>	commandBuffer = std::make_unique<CommandBuffer>(_ftDevice);
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

void ft::Application::updateScene(int key) {
	if (key == _ftWindow->KEY(KeyboardKeys::KEY_UP)) {
		_ftScene->getCamera()->vRotate(-10.0f);
	} else if (key == _ftWindow->KEY(KeyboardKeys::KEY_DOWN)) {
		_ftScene->getCamera()->vRotate(10.0f);
	} else if (key == _ftWindow->KEY(KeyboardKeys::KEY_RIGHT)) {
		_ftScene->getCamera()->hRotate(10.0f);
	} else if (key == _ftWindow->KEY(KeyboardKeys::KEY_LEFT)) {
		_ftScene->getCamera()->hRotate(-10.0f);
	} else if (key == _ftWindow->KEY(KeyboardKeys::KEY_R)) {
		_ftScene->getCamera()->hardSet({5,-1,0}, {1,-1,0}, {0,1,0});
	} else if (key == _ftWindow->KEY(KeyboardKeys::KEY_W)) {
		_ftScene->getCamera()->forward(0.5f);
	} else if (key == _ftWindow->KEY(KeyboardKeys::KEY_Q)){
		_ftScene->getCamera()->translateUp(0.5f);
	} else if (key == _ftWindow->KEY(KeyboardKeys::KEY_E)) {
		_ftScene->getCamera()->translateUp(-0.5f);
	} else if (key == _ftWindow->KEY(KeyboardKeys::KEY_A)) {
		_ftScene->getCamera()->translateSide(-0.5f);
	} else if (key == _ftWindow->KEY(KeyboardKeys::KEY_S)) {
		_ftScene->getCamera()->forward(-0.5f);
	} else if (key == _ftWindow->KEY(KeyboardKeys::KEY_D)) {
		_ftScene->getCamera()->translateSide(0.5f);
	} else if (key == _ftWindow->KEY(KeyboardKeys::KEY_KP_7)) {
		_ftScene->getCamera()->rotateWorldX(10.0f);
	} else if (key == _ftWindow->KEY(KeyboardKeys::KEY_KP_9)) {
		_ftScene->getCamera()->rotateWorldX(-10.0f);
	} else if (key == _ftWindow->KEY(KeyboardKeys::KEY_KP_4)) {
		_ftScene->getCamera()->rotateWorldY(10.0f);
	} else if (key == _ftWindow->KEY(KeyboardKeys::KEY_KP_6)) {
		_ftScene->getCamera()->rotateWorldY(-10.0f);
	} else if (key == _ftWindow->KEY(KeyboardKeys::KEY_KP_1)) {
		_ftScene->getCamera()->rotateWorldZ(10.0f);
	} else if (key == _ftWindow->KEY(KeyboardKeys::KEY_KP_3)) {
		_ftScene->getCamera()->rotateWorldZ(-10.0f);
	} else if (key == _ftWindow->KEY(KeyboardKeys::KEY_Z)) {

	}
	_ftScene->updateCameraUBO();
}

void ft::Application::initPushConstants() {
	_push.lightColor = {1.0f, 1.0f, 1.0f};
	_push.lightDirection = {10.0f, -5.0f, 1.0f};
	_push.ambient = 0.2f;
}
