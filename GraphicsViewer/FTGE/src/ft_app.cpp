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
	_ftRenderer = std::make_shared<ft::Renderer>(_ftWindow, _ftSurface, _ftPhysicalDevice, _ftDevice);


	_ftImageBuilder = std::make_shared<ImageBuilder>();
	_ftBufferBuilder = std::make_shared<BufferBuilder>();


	_ftGui = std::make_shared<Gui>(_ftInstance, _ftPhysicalDevice, _ftDevice,
								   _ftWindow, _ftRenderer->getRenderPass(), MAX_FRAMES_IN_FLIGHT);

	_ftDescriptorPool = std::make_shared<ft::DescriptorPool>(_ftDevice);

	_ftSimpleRdrSys = std::make_shared<ft::SimpleRdrSys>(_ftDevice, _ftRenderer, _ftDescriptorPool);
	_ftSimpleRdrSys->populateUBODescriptors(_ftRenderer->getUniformBuffers());

	createTextureImage();
//	createDescriptorSets();
//	createGraphicsPipeline();
}

void ft::Application::createScene() {
	CameraBuilder cameraBuilder;
	_ftScene = std::make_shared<Scene>(_ftDevice, _ftRenderer->getUniformBuffers());
	_ftScene->setCamera(cameraBuilder.setEyePosition({5,-1,0})
				.setTarget({1,-1,0})
				.setUpDirection({0,1,0})
				.setFOV(90)
				.setZNearFar(0.5f, 30.0f)
				.setAspect(_ftRenderer->getSwapChain()->getAspect())
				.build());
	_ftScene->setGeneralLight({1.0f,1.0f,1.0f}, {10.0, -50.0, 10.0}, 0.2f);
	ft::InstanceData data{};
	uint32_t  id;	(void) id;

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
	data.color = {0.9f, 0.9f, 0.9f};
	data.normalMatrix = glm::mat4(1.0f);
//	data.model = glm::scale(data.model, {10, 10, 10});
	data.model = glm::rotate(data.model, glm::radians(90.0f), {1,0,0});
	id = _ftScene->addObjectToTheScene("models/viking_room.obj", data);

//	data.model = glm::rotate(glm::mat4(1.0f), glm::radians(0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
//	data.model = glm::translate(data.model, glm::vec3{1.0f, -2.0f, 0.0f});
//	data.color = {0.5f, 0.95f, 0.2f};
//	id = _ftScene->addObjectCopyToTheScene(id, data);

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

//	data.model = glm::mat4(1.0f);
//	data.color = {0.95f, 0.2f, 0.0f};
//	data.normalMatrix = glm::mat4(1.0f);
//	id = _ftScene->addObjectToTheScene("models/sphere.mtl.obj", data);
//
//	data.model = glm::rotate(glm::mat4(1.0f), glm::radians(0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
//	data.model = glm::translate(data.model, glm::vec3{1.0f, -2.0f, 0.0f});
//	data.color = {0.5f, 0.95f, 0.2f};
//	id = _ftScene->addObjectCopyToTheScene(id, data);

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
	_ftRenderer->getSwapChain().reset();
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
	pipelineConfig.viewport.width = (float) _ftRenderer->getSwapChain()->getVKSwapChainExtent().width;
	pipelineConfig.viewport.height = (float) _ftRenderer->getSwapChain()->getVKSwapChainExtent().height;

	pipelineConfig.scissor.offset = {0, 0};
	pipelineConfig.scissor.extent = _ftRenderer->getSwapChain()->getVKSwapChainExtent();

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

	_ftGraphicsPipeline = std::make_shared<ft::GraphicsPipeline>(_ftDevice, _ftRenderer->getRenderPass(),
																 _ftDescriptorSets[0]->getDescriptorSetLayout()->getVKLayout(),
																 pipelineConfig);
}

// draw a frame
void ft::Application::drawFrame() {
	CommandBuffer::pointer commandBuffer;
	uint32_t index;
	std::tie(index, commandBuffer) = _ftRenderer->beginFrame();
	if (!commandBuffer) return;

	// begin the render pass
	_ftRenderer->beginRenderPass(commandBuffer, index);

	// recording the command buffer
	// for each pipeline

	// bind the graphics pipeline
//	vkCmdBindPipeline(commandBuffer->getVKCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, _ftGraphicsPipeline->getVKPipeline());
	vkCmdBindPipeline(commandBuffer->getVKCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, _ftSimpleRdrSys->getGraphicsPipeline()->getVKPipeline());
	// bind the descriptor sets
//	vkCmdBindDescriptorSets(commandBuffer->getVKCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS,
//							_ftGraphicsPipeline->getVKPipelineLayout(), 0, 1, &(_ftDescriptorSets[_currentFrame]->getVKDescriptorSet()),
//							0, nullptr);

	vkCmdBindDescriptorSets(commandBuffer->getVKCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS,
							_ftSimpleRdrSys->getGraphicsPipeline()->getVKPipelineLayout(), 0, 1,
							&(_ftSimpleRdrSys->getDescriptorSets()[_currentFrame]->getVKDescriptorSet()),
							0, nullptr);

	// scene
//	_ftScene->drawScene(commandBuffer, _ftGraphicsPipeline, _currentFrame);
	_ftScene->drawScene(commandBuffer, _ftSimpleRdrSys->getGraphicsPipeline(), _currentFrame);

	// gui
	_ftGui->render(commandBuffer);

	// end the render pass
	_ftRenderer->endRenderPass(commandBuffer, index);
	// end the frame
	_ftRenderer->endFrame(commandBuffer, index);
}

// create Descriptor sets
void ft::Application::createDescriptorSets() {
	_ftDescriptorSets.resize(ft::MAX_FRAMES_IN_FLIGHT);

	// create the descriptor set layout
	DescriptorSetLayoutBuilder dslBuilder;
	auto layout = dslBuilder.addDescriptorBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
				.addDescriptorBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
				.build(_ftDevice);

	for(auto& set : _ftDescriptorSets)
		set = _ftDescriptorPool->allocateSet(layout);

	// populate every descriptor
	for (uint32_t i = 0; i < ft::MAX_FRAMES_IN_FLIGHT; ++i) {

		// this is for ubo descriptor
		_ftDescriptorSets[i]->updateDescriptorBuffer(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
													 _ftRenderer->getUniformBuffers()[i], 0);

		// this is for sampler descriptor
		_ftDescriptorSets[i]->updateDescriptorImage(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
													VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
													_ftTextureImage, _ftRenderer->getSampler());
	}
}

// load images and create texture
void ft::Application::createTextureImage() {
	// load the image file
	int texWidth, texHeight, texChannels;
	stbi_uc *pixels = stbi_load(TEXTURE_PATH.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
	VkDeviceSize imageSize = texWidth * texHeight * 4;

	uint32_t mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(texWidth, texHeight)))) + 1;

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
			.setMipLevel(mipLevels)
			.setSampleCount(VK_SAMPLE_COUNT_1_BIT)
			.setFormat(VK_FORMAT_R8G8B8A8_SRGB)
			.setTiling(VK_IMAGE_TILING_OPTIMAL)
			.setUsageFlags(VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT)
			.setMemoryProperties(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
			.setAspectFlags(VK_IMAGE_ASPECT_COLOR_BIT)
			.build(_ftDevice);

	// transition the image layout for dst copy
	Image::transitionImageLayout(_ftDevice, _ftTextureImage->getVKImage(), VK_FORMAT_R8G8B8A8_SRGB,
						  VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, mipLevels);

	// copy the buffer to the image
	stagingBuffer->copyToImage(_ftTextureImage, texWidth, texHeight);

	// generate mip maps
//	generateMipmaps(_ftTextureImage->getVKImage(), VK_FORMAT_R8G8B8A8_SRGB, texWidth, texHeight, mipLevels);
	_ftTextureImage->generateMipmaps(VK_FORMAT_R8G8B8A8_SRGB);
	// clean up
	stbi_image_free(pixels);
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

