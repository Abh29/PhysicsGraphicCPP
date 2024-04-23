#ifndef FTGRAPHICS_FT_ATTACHMENT_H
#define FTGRAPHICS_FT_ATTACHMENT_H

#include "ft_device.h"
#include "ft_headers.h"
#include "ft_image.h"

namespace ft {

class Device;

class Attachment {

public:
  using pointer = std::shared_ptr<Attachment>;

  Attachment(VkAttachmentDescription description,
             VkAttachmentReference reference,
             VkImageView imageView = VK_NULL_HANDLE);
  ~Attachment() = default;
  Attachment(const Attachment &other) = delete;
  Attachment operator=(const Attachment &other) = delete;

  void setAttachmentIndex(uint32_t attachment);
  void setAttachmentImage(VkImageView imageView);
  [[nodiscard]] VkAttachmentDescription getVKAttachmentDescription() const;
  [[nodiscard]] VkAttachmentReference getVKAttachmentReference() const;
  [[nodiscard]] VkImageView getVKImageView() const;

private:
  VkAttachmentDescription _attachmentDescription;
  VkAttachmentReference _attachmentReference;
  VkImageView _attachmentImageView;
};

class AttachmentBuilder {
public:
  AttachmentBuilder();
  ~AttachmentBuilder() = default;

  AttachmentBuilder &setDescriptionFormat(VkFormat format);
  AttachmentBuilder &setDescriptionSamples(VkSampleCountFlagBits samples);
  AttachmentBuilder &setDescriptionLoadOp(VkAttachmentLoadOp loadOp);
  AttachmentBuilder &setDescriptionStoreOp(VkAttachmentStoreOp storeOp);
  AttachmentBuilder &setDescriptionStencilLoadOp(VkAttachmentLoadOp loadOp);
  AttachmentBuilder &setDescriptionStencilStoreOp(VkAttachmentStoreOp storeOp);
  AttachmentBuilder &setDescriptionInitialLayout(VkImageLayout layout);
  AttachmentBuilder &setDescriptionFinalLayout(VkImageLayout layout);
  AttachmentBuilder &setReferenceImageLayout(VkImageLayout layout);
  Attachment::pointer build();

private:
  VkAttachmentReference _attachmentReference;
  VkAttachmentDescription _attachmentDescription;
};

} // namespace ft

#endif // FTGRAPHICS_FT_ATTACHMENT_H
