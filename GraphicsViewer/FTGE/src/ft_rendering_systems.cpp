#include "../includes/ft_rendering_systems.h"

ft::RenderingSystem::RenderingSystem(Device::pointer device, Renderer::pointer renderer, DescriptorPool::pointer pool) :
_ftDevice(std::move(device)), _ftRenderer(std::move(renderer)), _ftDescriptorPool(std::move(pool)){}

ft::GraphicsPipeline::pointer ft::RenderingSystem::getGraphicsPipeline() const {return _ftPipeline;}

ft::DescriptorPool::pointer ft::RenderingSystem::getDescriptorPool() const {return _ftDescriptorPool;}

ft::DescriptorSetLayout::pointer ft::RenderingSystem::getDescriptorSetLayout() const {return _ftDescriptorSetLayout;}

std::vector<ft::DescriptorSet::pointer> ft::RenderingSystem::getDescriptorSets() const {return _ftDescriptorSets;}

/*********************************SimpleRdrSys****************************************/

ft::SimpleRdrSys::SimpleRdrSys(Device::pointer device, Renderer::pointer renderer, ft::DescriptorPool::pointer pool) :
RenderingSystem(std::move(device), std::move(renderer), std::move(pool))
{
	createDescriptors();
	createGraphicsPipeline();
}

void ft::SimpleRdrSys::createDescriptors() {
	_ftDescriptorSets.resize(ft::MAX_FRAMES_IN_FLIGHT);

	// create the descriptor set layout
	DescriptorSetLayoutBuilder dslBuilder;
	_ftDescriptorSetLayout = dslBuilder.addDescriptorBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
			.build(_ftDevice);

	for(auto& set : _ftDescriptorSets)
		set = _ftDescriptorPool->allocateSet(_ftDescriptorSetLayout);
}

void ft::SimpleRdrSys::populateUBODescriptors(std::vector<ft::Buffer::pointer> ubos) {
	assert(ubos.size() >= _ftDescriptorSets.size());

	for (uint32_t i = 0; i < _ftDescriptorSets.size(); ++i)
		_ftDescriptorSets[i]->updateDescriptorBuffer(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, ubos[i], 0);
}

void ft::SimpleRdrSys::createGraphicsPipeline() {
	ft::PipelineConfig pipelineConfig{};

	// shader modules
	pipelineConfig.vertShaderPath = "shaders/SimpleRdrSys.vert.spv";
	pipelineConfig.fragShaderPath = "shaders/SimpleRdrSys.frag.spv";
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

	_ftPipeline = std::make_shared<ft::GraphicsPipeline>(_ftDevice, _ftRenderer->getRenderPass(),
														 _ftDescriptorSetLayout->getVKLayout(),
														 pipelineConfig);
}

/**********************************TexturedRdrSys***************************************/



ft::TexturedRdrSys::TexturedRdrSys(Device::pointer device, Renderer::pointer renderer, ft::DescriptorPool::pointer pool) :
		RenderingSystem(std::move(device), std::move(renderer), std::move(pool))
{
	createDescriptors();
	createGraphicsPipeline();
}

void ft::TexturedRdrSys::createDescriptors() {
	_ftDescriptorSets.resize(ft::MAX_FRAMES_IN_FLIGHT);

	// create the descriptor set layout
	DescriptorSetLayoutBuilder dslBuilder;
	_ftDescriptorSetLayout = dslBuilder.addDescriptorBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
			.addDescriptorBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
			.build(_ftDevice);

	for(auto& set : _ftDescriptorSets)
		set = _ftDescriptorPool->allocateSet(_ftDescriptorSetLayout);
}

void ft::TexturedRdrSys::populateUBODescriptors(std::vector<ft::Buffer::pointer> ubos) {
	assert(ubos.size() >= _ftDescriptorSets.size());

	for (uint32_t i = 0; i < _ftDescriptorSets.size(); ++i)
		_ftDescriptorSets[i]->updateDescriptorBuffer(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, ubos[i], 0);
}

void ft::TexturedRdrSys::populateTextureDescriptors(Image::pointer image, Sampler::pointer sampler) {

	_ftSampler = std::move(sampler);
	_ftTextureImage = std::move(image);

	for (auto & _ftDescriptorSet : _ftDescriptorSets)
	_ftDescriptorSet->updateDescriptorImage(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
												VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
												_ftTextureImage, _ftSampler);

}

void ft::TexturedRdrSys::createGraphicsPipeline() {
	ft::PipelineConfig pipelineConfig{};

	// shader modules
	pipelineConfig.vertShaderPath = "shaders/TexturedRdrSys.vert.spv";
	pipelineConfig.fragShaderPath = "shaders/TexturedRdrSys.frag.spv";
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

	_ftPipeline = std::make_shared<ft::GraphicsPipeline>(_ftDevice, _ftRenderer->getRenderPass(),
														 _ftDescriptorSetLayout->getVKLayout(),
														 pipelineConfig);
}


