#include "../include.h"
#include "../includes/ft_renderPass.h"


ft::SubPass::SubPass(VkPipelineBindPoint bindPoint) : _subpassDescription{} {
	_subpassDescription.pipelineBindPoint = bindPoint;
}

void ft::SubPass::addColorAttachment(Attachment::pointer &attachment) {
	_colorAttachments.push_back(attachment->getVKAttachmentReference());
	_subpassDescription.colorAttachmentCount = _colorAttachments.size();
	_subpassDescription.pColorAttachments = _colorAttachments.data();
}

void ft::SubPass::addResolveAttachment(Attachment::pointer &attachment) {
	_resolveAttachments.push_back(attachment->getVKAttachmentReference());
	_subpassDescription.pResolveAttachments = _resolveAttachments.data();
}

void ft::SubPass::setDepthStencilAttachment(Attachment::pointer &attachment) {
	_depthStencilAttachment = attachment->getVKAttachmentReference();
	_subpassDescription.pDepthStencilAttachment = &_depthStencilAttachment;
}

void ft::SubPass::addInputAttachment(Attachment::pointer &attachment) {
	_inputAttachments.push_back(attachment->getVKAttachmentReference());
	_subpassDescription.inputAttachmentCount = _inputAttachments.size();
	_subpassDescription.pInputAttachments = _inputAttachments.data();
}

void ft::SubPass::addPreservedAttachment(Attachment::pointer &attachment) {
	_preservedAttachments.push_back(attachment->getVKAttachmentReference().attachment);
	_subpassDescription.preserveAttachmentCount = _preservedAttachments.size();
	_subpassDescription.pPreserveAttachments = _preservedAttachments.data();
}

VkSubpassDescription ft::SubPass::getVKSubpassDescription() const {
	return _subpassDescription;
}

/****************************Subpass Dependency********************************/


ft::SubpassDependency::SubpassDependency(uint32_t srcSubpass, uint32_t dstSubpass) : _subpassDependency{} {
	_subpassDependency.srcSubpass = srcSubpass;
	_subpassDependency.dstSubpass = dstSubpass;
}

ft::SubpassDependency &ft::SubpassDependency::setSrcStageMask(VkPipelineStageFlags srcStageMask) {
	_subpassDependency.srcStageMask = srcStageMask;
	return *this;
}

ft::SubpassDependency &ft::SubpassDependency::setSrcAccessMask(VkAccessFlags srcAccessMask) {
	_subpassDependency.srcAccessMask = srcAccessMask;
	return *this;
}

ft::SubpassDependency &ft::SubpassDependency::setDstStageMask(VkPipelineStageFlags dstStageMask) {
	_subpassDependency.dstStageMask = dstStageMask;
	return *this;
}

ft::SubpassDependency &ft::SubpassDependency::setDstAccessMask(VkAccessFlags dstAccessMask) {
	_subpassDependency.dstAccessMask = dstAccessMask;
	return *this;
}

VkSubpassDependency ft::SubpassDependency::getVKSubpassDependency() const {
	return _subpassDependency;
}

/**************************************RenderPass*********************************/


ft::RenderPass::RenderPass(ft::Device::pointer &device) :
_renderPassCreateInfo{}, _renderPassCreated{false}, _ftDevice(device) {
	_renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
}

ft::RenderPass::~RenderPass() {
	if (_renderPassCreated)
		vkDestroyRenderPass(_ftDevice->getVKDevice(), _renderPass, nullptr);
}

void ft::RenderPass::create() {
	if (_renderPassCreated)
		throw std::runtime_error("Can not create a render pass that has already been created!");

	auto attachments = getAttachmentsDescriptions();
	auto subpasses = getSubpassDescriptions();
	auto dependencies = getSubpassDependencies();

	_renderPassCreateInfo.attachmentCount = _ftAttachments.size();
	_renderPassCreateInfo.pAttachments = attachments.data();
	_renderPassCreateInfo.subpassCount = _ftSubpasses.size();
	_renderPassCreateInfo.pSubpasses = subpasses.data();
	_renderPassCreateInfo.dependencyCount = _ftDependencies.size();
	_renderPassCreateInfo.pDependencies = dependencies.data();

	if (vkCreateRenderPass(_ftDevice->getVKDevice(), &_renderPassCreateInfo, nullptr, &_renderPass) != VK_SUCCESS) {
		throw std::runtime_error("failed to create a render pass!");
	}
	_renderPassCreated = true;
}

VkRenderPassCreateInfo ft::RenderPass::getVKRenderPassCreateInfo() const {
	return _renderPassCreateInfo;
}

VkRenderPass ft::RenderPass::getVKRenderPass() const {
	return _renderPass;
}

bool ft::RenderPass::isCreated() const {
	return _renderPassCreated;
}

void ft::RenderPass::addSubpass(ft::SubPass::pointer &subPass) {
	_ftSubpasses.push_back(subPass);
}


void ft::RenderPass::addSubpassDependency(ft::SubpassDependency::pointer &dependency) {
	_ftDependencies.push_back(dependency);
}

std::vector<VkAttachmentDescription> ft::RenderPass::getAttachmentsDescriptions() const {
	std::vector<VkAttachmentDescription> out(_ftAttachments.size());
	std::transform(_ftAttachments.begin(), _ftAttachments.end(), out.begin(),
				   [](const ft::Attachment::pointer &p) {return p->getVKAttachmentDescription();});
	return out;
}

std::vector<VkSubpassDependency> ft::RenderPass::getSubpassDependencies() const {
	std::vector<VkSubpassDependency> out(_ftDependencies.size());
	std::transform(_ftDependencies.begin(), _ftDependencies.end(), out.begin(),
				   [](const ft::SubpassDependency::pointer &p) {return p->getVKSubpassDependency();});
	return out;
}

std::vector<VkSubpassDescription> ft::RenderPass::getSubpassDescriptions() const {
	std::vector<VkSubpassDescription> out(_ftSubpasses.size());
	std::transform(_ftSubpasses.begin(), _ftSubpasses.end(), out.begin(),
				   [](const ft::SubPass::pointer &p) {return p->getVKSubpassDescription();});
	return out;
}

std::vector<VkImageView> ft::RenderPass::getAttachmentImageViews() const {
	std::vector<VkImageView> out(_ftAttachments.size());
	std::transform(_ftAttachments.begin(), _ftAttachments.end(), out.begin(),
				   [](const Attachment::pointer &p) {return p->getVKImageView();});
	return out;
}


void ft::RenderPass::addColorAttachmentToSubpass(ft::Attachment::pointer &attachment, uint32_t subpassIndex) {
	attachment->setAttachmentIndex(_ftAttachments.size());
	_ftAttachments.push_back(attachment);
	_ftSubpasses[subpassIndex]->addColorAttachment(attachment);
}

void ft::RenderPass::addInputAttachmentToSubpass(ft::Attachment::pointer &attachment, uint32_t subpassIndex) {
	attachment->setAttachmentIndex(_ftAttachments.size());
	_ftAttachments.push_back(attachment);
	_ftSubpasses[subpassIndex]->addInputAttachment(attachment);
}

void ft::RenderPass::addResolveAttachmentToSubpass(ft::Attachment::pointer &attachment, uint32_t subpassIndex) {
	attachment->setAttachmentIndex(_ftAttachments.size());
	_ftAttachments.push_back(attachment);
	_ftSubpasses[subpassIndex]->addResolveAttachment(attachment);
}

void ft::RenderPass::addPreservedAttachmentToSubpass(ft::Attachment::pointer &attachment, uint32_t subpassIndex) {
	_ftSubpasses[subpassIndex]->addPreservedAttachment(attachment);
}

void ft::RenderPass::setDepthStencilAttachmentToSubpass(ft::Attachment::pointer &attachment, uint32_t subpassIndex) {
	attachment->setAttachmentIndex(_ftAttachments.size());
	_ftAttachments.push_back(attachment);
	_ftSubpasses[subpassIndex]->setDepthStencilAttachment(attachment);
}

void ft::RenderPass::appendImageToAttachment(VkImageView imageView, uint32_t index) {
	_ftAttachments[index]->setAttachmentImage(imageView);
}
