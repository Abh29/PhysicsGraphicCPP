#ifndef FTGRAPHICS_FT_RENDERPASS_H
#define FTGRAPHICS_FT_RENDERPASS_H

#include "ft_attachment.h"
#include "ft_headers.h"

namespace ft {

class SubPass {
public:
  using pointer = std::shared_ptr<SubPass>;

  SubPass(VkPipelineBindPoint bindPoint);
  ~SubPass() = default;

  void addColorAttachment(Attachment::pointer &attachment);
  void addResolveAttachment(Attachment::pointer &attachment);
  void setDepthStencilAttachment(Attachment::pointer &attachment);
  void addInputAttachment(Attachment::pointer &attachment);
  void addPreservedAttachment(Attachment::pointer &attachment);
  [[nodiscard]] VkSubpassDescription getVKSubpassDescription() const;

private:
  VkSubpassDescription _subpassDescription;
  std::vector<VkAttachmentReference> _colorAttachments;
  std::vector<VkAttachmentReference> _resolveAttachments;
  std::vector<VkAttachmentReference> _inputAttachments;
  std::vector<uint32_t> _preservedAttachments;
  VkAttachmentReference _depthStencilAttachment;
};

class SubpassDependency {

public:
  using pointer = std::shared_ptr<SubpassDependency>;

  SubpassDependency(uint32_t srcSubpass, uint32_t dstSubpass);
  ~SubpassDependency() = default;

  SubpassDependency &setSrcStageMask(VkPipelineStageFlags srcStageMask);
  SubpassDependency &setSrcAccessMask(VkAccessFlags srcAccessMask);
  SubpassDependency &setDstStageMask(VkPipelineStageFlags dstStageMask);
  SubpassDependency &setDstAccessMask(VkAccessFlags dstAccessMask);

  [[nodiscard]] VkSubpassDependency getVKSubpassDependency() const;

private:
  VkSubpassDependency _subpassDependency;
};

class RenderPass {

public:
  using pointer = std::shared_ptr<RenderPass>;

  RenderPass(Device::pointer &device);
  ~RenderPass();

  RenderPass(const RenderPass &other) = delete;
  RenderPass operator=(const RenderPass &other) = delete;

  void create();

  [[nodiscard]] VkRenderPassCreateInfo getVKRenderPassCreateInfo() const;
  [[nodiscard]] VkRenderPass getVKRenderPass() const;
  bool isCreated() const;

  void addSubpass(SubPass::pointer &subPass);
  void addColorAttachmentToSubpass(Attachment::pointer &attachment,
                                   uint32_t subpassIndex = 0);
  void addInputAttachmentToSubpass(Attachment::pointer &attachment,
                                   uint32_t subpassIndex = 0);
  void addResolveAttachmentToSubpass(Attachment::pointer &attachment,
                                     uint32_t subpassIndex = 0);
  void addPreservedAttachmentToSubpass(Attachment::pointer &attachment,
                                       uint32_t subpassIndex = 0);
  void setDepthStencilAttachmentToSubpass(Attachment::pointer &attachment,
                                          uint32_t subpassIndex = 0);
  void addSubpassDependency(SubpassDependency::pointer &dependency);
  void appendImageToAttachment(VkImageView imageView, uint32_t index);

private:
  std::vector<VkAttachmentDescription> getAttachmentsDescriptions() const;
  std::vector<VkSubpassDependency> getSubpassDependencies() const;
  std::vector<VkSubpassDescription> getSubpassDescriptions() const;
  std::vector<VkImageView> getAttachmentImageViews() const;

  VkRenderPassCreateInfo _renderPassCreateInfo;
  bool _renderPassCreated;
  VkRenderPass _renderPass = VK_NULL_HANDLE;
  Device::pointer _ftDevice;
  std::vector<Attachment::pointer> _ftAttachments;
  std::vector<SubpassDependency::pointer> _ftDependencies;
  std::vector<SubPass::pointer> _ftSubpasses;
};

} // namespace ft

#endif // FTGRAPHICS_FT_RENDERPASS_H
