#ifndef FTGRAPHICS_FT_RENDERING_SYSTEMS_H
#define FTGRAPHICS_FT_RENDERING_SYSTEMS_H

#include "ft_defines.h"
#include "ft_descriptor.h"
#include "ft_device.h"
#include "ft_headers.h"
#include "ft_pipeline.h"
#include "ft_renderer.h"
#include "ft_texture.h"
#include "ft_vertex.h"

namespace ft {

class RenderingSystem {

public:
  using pointer = std::shared_ptr<RenderingSystem>;

  RenderingSystem(Device::pointer, Renderer::pointer, DescriptorPool::pointer);
  virtual ~RenderingSystem() = default;

  [[nodiscard]] GraphicsPipeline::pointer getGraphicsPipeline() const;
  [[nodiscard]] DescriptorSetLayout::pointer getDescriptorSetLayout() const;
  [[nodiscard]] DescriptorPool::pointer getDescriptorPool() const;

protected:
  Device::pointer _ftDevice;
  Renderer::pointer _ftRenderer;
  DescriptorPool::pointer _ftDescriptorPool;
  DescriptorSetLayout::pointer _ftDescriptorSetLayout;
  GraphicsPipeline::pointer _ftPipeline;
};

class InstanceRdrSys final : public RenderingSystem {

public:
  using pointer = std::shared_ptr<InstanceRdrSys>;

  explicit InstanceRdrSys(Device::pointer, Renderer::pointer,
                          DescriptorPool::pointer);
  ~InstanceRdrSys() override = default;

  void populateUBODescriptors(std::vector<Buffer::pointer> ubos);
  [[nodiscard]] std::vector<DescriptorSet::pointer> getDescriptorSets() const;

private:
  void createDescriptors();
  void createGraphicsPipeline();

  std::vector<DescriptorSet::pointer> _ftDescriptorSets;
};

class SimpleRdrSys final : public RenderingSystem {

public:
  using pointer = std::shared_ptr<SimpleRdrSys>;

  explicit SimpleRdrSys(Device::pointer, Renderer::pointer,
                        DescriptorPool::pointer);
  ~SimpleRdrSys() override = default;

  void populateUBODescriptors(std::vector<Buffer::pointer> ubos);
  [[nodiscard]] std::vector<DescriptorSet::pointer> getDescriptorSets() const;

private:
  void createDescriptors();
  void createGraphicsPipeline();

  std::vector<DescriptorSet::pointer> _ftDescriptorSets;
};

class OneTextureRdrSys final : public RenderingSystem {
public:
  using pointer = std::shared_ptr<OneTextureRdrSys>;

  OneTextureRdrSys(Device::pointer, Renderer::pointer, DescriptorPool::pointer);
  ~OneTextureRdrSys() override = default;

  [[nodiscard]] uint32_t getTextureImageBinding() const;
  [[nodiscard]] uint32_t getSamplerBinding() const;

private:
  void createDescriptorSetLayout();
  void createGraphicsPipeline();

  uint32_t _samplerBinding;
  uint32_t _textureImageBinding;
};

class TwoTextureRdrSys final : public RenderingSystem {
public:
  using pointer = std::shared_ptr<TwoTextureRdrSys>;

  TwoTextureRdrSys(Device::pointer, Renderer::pointer, DescriptorPool::pointer);
  ~TwoTextureRdrSys() override = default;

  [[nodiscard]] std::vector<DescriptorSet::pointer> getDescriptorSets() const;
  [[nodiscard]] uint32_t getNormalsImageBinding() const;
  [[nodiscard]] uint32_t getTextureImageBinding() const;
  [[nodiscard]] uint32_t getUboBinding() const;

private:
  void createDescriptorSetLayout();
  void createGraphicsPipeline();

  uint32_t _uboBinding;
  uint32_t _textureImageBinding;
  uint32_t _normalsImageBinding;
};

class PickingRdrSys final : public RenderingSystem {
public:
  using pointer = std::shared_ptr<PickingRdrSys>;

  PickingRdrSys(Device::pointer, Renderer::pointer, DescriptorPool::pointer);
  ~PickingRdrSys() override = default;

  void populateUBODescriptors(std::vector<Buffer::pointer> ubos);
  [[nodiscard]] std::vector<DescriptorSet::pointer> getDescriptorSets() const;
  void createGraphicsPipeline(const RenderPass::pointer &renderPass);

private:
  void createDescriptorSetLayout();

  std::vector<DescriptorSet::pointer> _ftDescriptorSets;
  bool _isCreatedPipeline = false;
};

class SkyBoxRdrSys final : public RenderingSystem {
public:
  using pointer = std::shared_ptr<SkyBoxRdrSys>;

  SkyBoxRdrSys(Device::pointer, Renderer::pointer, DescriptorPool::pointer);
  ~SkyBoxRdrSys() override = default;

  [[nodiscard]] uint32_t getTextureImageBinding() const;
  [[nodiscard]] uint32_t getSamplerBinding() const;

private:
  void createDescriptorSetLayout();
  void createGraphicsPipeline();

  uint32_t _samplerBinding;
  uint32_t _textureImageBinding;
};

class OutlineRdrSys final : public RenderingSystem {
public:
  using pointer = std::shared_ptr<OutlineRdrSys>;

  OutlineRdrSys(Device::pointer, Renderer::pointer, DescriptorPool::pointer);
  ~OutlineRdrSys() override = default;

  [[nodiscard]] uint32_t getUboBinding() const;

private:
  void createDescriptorSetLayout();
  void createGraphicsPipeline();

  uint32_t _uboBinding;
};

class PointRdrSys final : public RenderingSystem {
public:
  using pointer = std::shared_ptr<PointRdrSys>;

  PointRdrSys(Device::pointer, Renderer::pointer, DescriptorPool::pointer);
  ~PointRdrSys() override = default;

  [[nodiscard]] uint32_t getUboBinding() const;

private:
  void createDescriptorSetLayout();
  void createGraphicsPipeline();

  uint32_t _uboBinding;
};

class LineRdrSys final : public RenderingSystem {
public:
  using pointer = std::shared_ptr<LineRdrSys>;

  LineRdrSys(Device::pointer, Renderer::pointer, DescriptorPool::pointer);
  ~LineRdrSys() override = default;

  [[nodiscard]] uint32_t getUboBinding() const;

private:
  void createDescriptorSetLayout();
  void createGraphicsPipeline();

  uint32_t _uboBinding;
};

class NormDebugRdrSys final : public RenderingSystem {
public:
  using pointer = std::shared_ptr<NormDebugRdrSys>;

  NormDebugRdrSys(Device::pointer, Renderer::pointer, DescriptorPool::pointer);
  ~NormDebugRdrSys() override = default;

  [[nodiscard]] uint32_t getVUboBinding() const;

private:
  void createDescriptorSetLayout();
  void createGraphicsPipeline();

  uint32_t _vertexUboBinding;
};

} // namespace ft

#endif // FTGRAPHICS_FT_RENDERING_SYSTEMS_H
