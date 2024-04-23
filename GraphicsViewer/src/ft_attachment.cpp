#include "../includes/ft_attachment.h"


ft::Attachment::Attachment(VkAttachmentDescription description, VkAttachmentReference reference,
						   VkImageView imageView) :
		_attachmentDescription{description}, _attachmentReference{reference},
		_attachmentImageView(imageView){}

VkAttachmentDescription ft::Attachment::getVKAttachmentDescription() const {
	return _attachmentDescription;
}

VkAttachmentReference ft::Attachment::getVKAttachmentReference() const {
	return _attachmentReference;
}

void ft::Attachment::setAttachmentIndex(uint32_t attachment) {
	_attachmentReference.attachment = attachment;
}

VkImageView ft::Attachment::getVKImageView() const {return _attachmentImageView;}

void ft::Attachment::setAttachmentImage(VkImageView imageView) {
	_attachmentImageView = imageView;
}

/*************************************Attachment Builder************************/

ft::AttachmentBuilder::AttachmentBuilder() : _attachmentReference{}, _attachmentDescription{} {

	_attachmentDescription.samples = VK_SAMPLE_COUNT_1_BIT;
	_attachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	_attachmentDescription.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	_attachmentDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	_attachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	_attachmentDescription.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	_attachmentDescription.finalLayout = VK_IMAGE_LAYOUT_UNDEFINED;

}

ft::AttachmentBuilder &ft::AttachmentBuilder::setDescriptionFormat(VkFormat format) {
	_attachmentDescription.format = format;
	return *this;
}

ft::AttachmentBuilder &ft::AttachmentBuilder::setDescriptionSamples(VkSampleCountFlagBits samples) {
	_attachmentDescription.samples = samples;
	return *this;
}

ft::AttachmentBuilder &ft::AttachmentBuilder::setDescriptionLoadOp(VkAttachmentLoadOp loadOp) {
	_attachmentDescription.loadOp = loadOp;
	return *this;
}

ft::AttachmentBuilder &ft::AttachmentBuilder::setDescriptionStoreOp(VkAttachmentStoreOp storeOp) {
	_attachmentDescription.storeOp = storeOp;
	return *this;
}

ft::AttachmentBuilder &ft::AttachmentBuilder::setDescriptionStencilLoadOp(VkAttachmentLoadOp loadOp) {
	_attachmentDescription.stencilLoadOp = loadOp;
	return *this;
}

ft::AttachmentBuilder &ft::AttachmentBuilder::setDescriptionStencilStoreOp(VkAttachmentStoreOp storeOp) {
	_attachmentDescription.stencilStoreOp = storeOp;
	return *this;
}

ft::AttachmentBuilder &ft::AttachmentBuilder::setDescriptionInitialLayout(VkImageLayout layout) {
	_attachmentDescription.initialLayout = layout;
	return *this;
}

ft::AttachmentBuilder &ft::AttachmentBuilder::setDescriptionFinalLayout(VkImageLayout layout) {
	_attachmentDescription.finalLayout = layout;
	return *this;
}

ft::AttachmentBuilder &ft::AttachmentBuilder::setReferenceImageLayout(VkImageLayout layout) {
	_attachmentReference.layout = layout;
	return *this;
}

ft::Attachment::pointer ft::AttachmentBuilder::build() {
	return std::make_shared<Attachment>(_attachmentDescription, _attachmentReference);
}
