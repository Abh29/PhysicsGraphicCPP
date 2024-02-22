#ifndef FTGRAPHICS_FT_COMMAND_H
#define FTGRAPHICS_FT_COMMAND_H

#include "ft_device.h"
#include "ft_headers.h"

namespace ft {

class CommandBuffer {

public:
  using pointer = std::shared_ptr<CommandBuffer>;

  CommandBuffer(Device::pointer device,
                VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY);
  ~CommandBuffer();

  [[nodiscard]] VkCommandBuffer getVKCommandBuffer() const;
  [[nodiscard]] const VkCommandBuffer *getVKCommandBufferPtr() const;
  void beginRecording(VkCommandBufferUsageFlags flags);
  void reset(VkCommandBufferResetFlags flags = 0);
  void end();
  void submit(VkQueue vkQueue, VkSubmitInfo submitInfo = {},
              VkFence fence = VK_NULL_HANDLE);

private:
  Device::pointer _ftDevice;
  VkCommandBuffer _commandBuffer;
  VkCommandBufferLevel _commandBufferLevel;
};
} // namespace ft

#endif // FTGRAPHICS_FT_COMMAND_H
