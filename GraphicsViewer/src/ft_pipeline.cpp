#include "../includes/ft_pipeline.h"

ft::GraphicsPipeline::GraphicsPipeline(
    Device::pointer device, RenderPass::pointer renderPass,
    VkDescriptorSetLayout descriptorSetLayout, PipelineConfig &config)
    : _ftDevice(device), _ftRenderPass(renderPass),
      _descriptorSetLayout(descriptorSetLayout) {

  createPipeline(config);
}

ft::GraphicsPipeline::~GraphicsPipeline() {
  vkDestroyPipeline(_ftDevice->getVKDevice(), _pipeline, nullptr);
  vkDestroyPipelineLayout(_ftDevice->getVKDevice(), _pipelineLayout, nullptr);
}

VkPipeline ft::GraphicsPipeline::getVKPipeline() const { return _pipeline; }

VkPipelineLayout ft::GraphicsPipeline::getVKPipelineLayout() const {
  return _pipelineLayout;
}

void ft::GraphicsPipeline::createPipeline(PipelineConfig &config) {
  _ftVertexShader =
      std::make_shared<ft::Shader>(_ftDevice, config.vertShaderPath,
                                   ft::Shader::ShaderType::FT_VERTEX_SHADER);
  _ftFragmentShader =
      std::make_shared<ft::Shader>(_ftDevice, config.fragShaderPath,
                                   ft::Shader::ShaderType::FT_FRAGMENT_SHADER);
  if (!config.geomShaderPath.empty())
    _ftGeometryShader = std::make_shared<ft::Shader>(
        _ftDevice, config.geomShaderPath,
        ft::Shader::ShaderType::FT_GEOMETRY_SHADER);

  std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
  // vertex shader stage
  VkPipelineShaderStageCreateInfo vertShaderStageCreateInfo{};
  vertShaderStageCreateInfo.sType =
      VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  vertShaderStageCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
  vertShaderStageCreateInfo.module = _ftVertexShader->getVKShaderModule();
  vertShaderStageCreateInfo.pName = config.vertShaderEntryPoint.c_str();
  vertShaderStageCreateInfo.pSpecializationInfo =
      nullptr; // for specifying constants
  shaderStages.push_back(vertShaderStageCreateInfo);

  // fragment shader stage
  VkPipelineShaderStageCreateInfo fragShaderStageCreationInfo{};
  fragShaderStageCreationInfo.sType =
      VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  fragShaderStageCreationInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
  fragShaderStageCreationInfo.module = _ftFragmentShader->getVKShaderModule();
  fragShaderStageCreationInfo.pName = config.vertShaderEntryPoint.c_str();
  fragShaderStageCreationInfo.pSpecializationInfo = nullptr;
  shaderStages.push_back(fragShaderStageCreationInfo);
  // geometry shader stage

  VkPipelineShaderStageCreateInfo geometryStageCreateInfo{};
  if (!config.geomShaderPath.empty()) {
    geometryStageCreateInfo.sType =
        VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    geometryStageCreateInfo.stage = VK_SHADER_STAGE_GEOMETRY_BIT;
    geometryStageCreateInfo.module = _ftGeometryShader->getVKShaderModule();
    geometryStageCreateInfo.pName = config.geomShaderEntryPoint.c_str();
    geometryStageCreateInfo.pSpecializationInfo = nullptr;
    shaderStages.push_back(geometryStageCreateInfo);
  }

  // dynamic state
  VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo{};
  dynamicStateCreateInfo.sType =
      VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
  dynamicStateCreateInfo.dynamicStateCount =
      static_cast<uint32_t>(config.dynamicStates.size());
  dynamicStateCreateInfo.pDynamicStates = config.dynamicStates.data();

  // vertex input state
  VkPipelineVertexInputStateCreateInfo vertexInputStateCreateInfo{};
  vertexInputStateCreateInfo.sType =
      VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
  vertexInputStateCreateInfo.vertexBindingDescriptionCount =
      static_cast<uint32_t>(config.bindings.size());
  vertexInputStateCreateInfo.pVertexBindingDescriptions =
      config.bindings.data();
  vertexInputStateCreateInfo.vertexAttributeDescriptionCount =
      static_cast<uint32_t>(config.attributes.size());
  vertexInputStateCreateInfo.pVertexAttributeDescriptions =
      config.attributes.data();

  // view port and scissors
  VkPipelineViewportStateCreateInfo viewportStateCreateInfo{};
  viewportStateCreateInfo.sType =
      VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
  viewportStateCreateInfo.viewportCount = 1;
  viewportStateCreateInfo.pViewports = &(config.viewport);
  viewportStateCreateInfo.scissorCount = 1;
  viewportStateCreateInfo.pScissors = &(config.scissor);

  // color blending
  config.colorBlendState.attachmentCount =
      static_cast<uint32_t>(config.colorBlendAttachments.size());
  config.colorBlendState.pAttachments = config.colorBlendAttachments.data();

  // pipeline layout
  VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{};
  pipelineLayoutCreateInfo.sType =
      VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  pipelineLayoutCreateInfo.setLayoutCount = 1;
  pipelineLayoutCreateInfo.pSetLayouts = &_descriptorSetLayout;
  pipelineLayoutCreateInfo.pushConstantRangeCount =
      static_cast<uint32_t>(config.pushConstantRanges.size());
  pipelineLayoutCreateInfo.pPushConstantRanges =
      config.pushConstantRanges.data();

  if (vkCreatePipelineLayout(_ftDevice->getVKDevice(),
                             &pipelineLayoutCreateInfo, nullptr,
                             &_pipelineLayout) != VK_SUCCESS) {
    throw std::runtime_error("failed to create a pipeline layout!");
  }

  // creating the pipeline
  VkGraphicsPipelineCreateInfo pipelineCreateInfo{};
  pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
  pipelineCreateInfo.stageCount = static_cast<uint32_t>(shaderStages.size());
  pipelineCreateInfo.pStages = shaderStages.data();
  pipelineCreateInfo.pVertexInputState = &vertexInputStateCreateInfo;
  pipelineCreateInfo.pInputAssemblyState = &(config.inputAssemblyState);
  pipelineCreateInfo.pViewportState = &viewportStateCreateInfo;
  pipelineCreateInfo.pRasterizationState = &(config.rasterizerState);
  pipelineCreateInfo.pMultisampleState = &(config.multisampleState);
  pipelineCreateInfo.pDepthStencilState = &(config.depthStencilState);
  pipelineCreateInfo.pColorBlendState = &(config.colorBlendState);
  pipelineCreateInfo.pDynamicState = &dynamicStateCreateInfo;
  pipelineCreateInfo.layout = _pipelineLayout;
  pipelineCreateInfo.renderPass = _ftRenderPass->getVKRenderPass();
  pipelineCreateInfo.subpass = 0; // index of sub pass
  pipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;
  pipelineCreateInfo.basePipelineIndex = -1;

  if (vkCreateGraphicsPipelines(_ftDevice->getVKDevice(), VK_NULL_HANDLE, 1,
                                &pipelineCreateInfo, nullptr,
                                &_pipeline) != VK_SUCCESS) {
    throw std::runtime_error("failed to create a graphics pipeline!");
  }
}
