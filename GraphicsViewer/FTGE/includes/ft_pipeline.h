#ifndef FTGRAPHICS_FT_PIPELINE_H
#define FTGRAPHICS_FT_PIPELINE_H

#include "ft_attachment.h"
#include "ft_defines.h"
#include "ft_descriptor.h"
#include "ft_device.h"
#include "ft_headers.h"
#include "ft_renderPass.h"
#include "ft_shader.h"
#include "ft_swapChain.h"

namespace ft {

struct PipelineConfig {
  std::string vertShaderPath;
  std::string fragShaderPath;
  std::string vertShaderEntryPoint = "main";
  std::string fragShaderEntryPoint = "main";
  std::vector<VkDynamicState> dynamicStates;
  VkPipelineDepthStencilStateCreateInfo depthStencilState;
  VkPipelineInputAssemblyStateCreateInfo inputAssemblyState;
  VkPipelineRasterizationStateCreateInfo rasterizerState;
  VkPipelineMultisampleStateCreateInfo multisampleState;
  std::vector<VkPipelineColorBlendAttachmentState> colorBlendAttachments;
  VkPipelineColorBlendStateCreateInfo colorBlendState;
  VkPipelineTessellationStateCreateInfo tesselationState;
  std::vector<VkVertexInputBindingDescription> bindings;
  std::vector<VkVertexInputAttributeDescription> attributes;
  VkViewport viewport;
  VkRect2D scissor;
  std::vector<VkPushConstantRange> pushConstantRanges;
};

class GraphicsPipeline {
public:
  using pointer = std::shared_ptr<GraphicsPipeline>;
  GraphicsPipeline(Device::pointer device, RenderPass::pointer renderPass,
                   VkDescriptorSetLayout descriptorSetLayout,
                   PipelineConfig &config);
  ~GraphicsPipeline();

  [[nodiscard]] VkPipeline getVKPipeline() const;
  [[nodiscard]] VkPipelineLayout getVKPipelineLayout() const;

private:
  void createPipeline(PipelineConfig &config);

  Device::pointer _ftDevice;
  RenderPass::pointer _ftRenderPass;
  VkDescriptorSetLayout _descriptorSetLayout;
  VkPipeline _pipeline;
  VkPipelineLayout _pipelineLayout;
  Shader::pointer _ftVertexShader;
  Shader::pointer _ftFragmentShader;
};

} // namespace ft

#endif // FTGRAPHICS_FT_PIPELINE_H
