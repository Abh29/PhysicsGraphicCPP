#ifndef FTGRAPHICS_FT_RENDERER_H
#define FTGRAPHICS_FT_RENDERER_H

#include "ft_buffer.h"
#include "ft_command.h"
#include "ft_defines.h"
#include "ft_device.h"
#include "ft_headers.h"
#include "ft_image.h"
#include "ft_physicalDevice.h"
#include "ft_pipeline.h"
#include "ft_renderPass.h"
#include "ft_sampler.h"
#include "ft_surface.h"
#include "ft_window.h"

namespace ft {

class Renderer {

public:
  using pointer = std::shared_ptr<Renderer>;

  Renderer(Window::pointer window, Surface::pointer surface,
           PhysicalDevice::pointer physicalDevice, Device::pointer device);

  ~Renderer();

  std::pair<uint32_t, CommandBuffer::pointer> beginFrame();
  void endFrame(const CommandBuffer::pointer &commandBuffer,
                uint32_t imageIndex);
  void beginRenderPass(const CommandBuffer::pointer &commandBuffer,
                       uint32_t imageIndex);
  void endRenderPass(const CommandBuffer::pointer &commandBuffer,
                     uint32_t imageIndex);
  CommandBuffer::pointer getCurrentCommandBuffer();
  [[nodiscard]] SwapChain::pointer getSwapChain() const;
  [[nodiscard]] RenderPass::pointer getRenderPass() const;
  [[nodiscard]] std::vector<Buffer::pointer> getUniformBuffers() const;

private:
  void initRenderer();
  void initRenderPasses();
  void recreateSwapChain();
  void createUniformBuffers();

  Window::pointer _ftWindow;
  Surface::pointer _ftSurface;
  PhysicalDevice::pointer _ftPhysicalDevice;
  Device::pointer _ftDevice;
  SwapChain::pointer _ftSwapChain;
  std::shared_ptr<ImageBuilder> _ftImageBuilder;
  std::shared_ptr<BufferBuilder> _ftBufferBuilder;
  std::vector<CommandBuffer::pointer> _ftCommandBuffers;
  RenderPass::pointer _ftRenderPass;
  uint32_t _currentFrame = 0;
  std::vector<Buffer::pointer> _ftUniformBuffers;
};

} // namespace ft

#endif // FTGRAPHICS_FT_RENDERER_H
