#include "../includes/ft_picker.h"


ft::MousePicker::MousePicker(Device::pointer device, uint32_t width, uint32_t height, ft::PickingRdrSys::pointer rdrSys):
_ftDevice(std::move(device)),
_width(width), _height(height),
_ftPickingRdrSys(std::move(rdrSys)){
    createPickingRenderPass();
    createPickingResources();
    createPickingSyncObjs();
    _ftCommandBuffer = std::make_shared<CommandBuffer>(_ftDevice);
    _ftPickingRdrSys->createGraphicsPipeline(_ftRenderPass);
}

ft::MousePicker::~MousePicker() {
    vkDestroyFence(_ftDevice->getVKDevice(), _fence, nullptr);
    vkDestroyFramebuffer(_ftDevice->getVKDevice(), _pickingFrameBuffer, nullptr);
}

uint32_t ft::MousePicker::pick(const ft::Scene::pointer &scene, uint32_t x, uint32_t y) {
    if (_viewUpdated) {
        drawIDs(scene);
        _viewUpdated = false;
    }
    auto pixels = reinterpret_cast<uint32_t*>(_ftColorBuffer->getMappedData());
    return pixels[(int)y * _width + (int)x];
}

void ft::MousePicker::notifyUpdatedView() {_viewUpdated = true;}

VkFence ft::MousePicker::getVkFence() const {return _fence;}

void ft::MousePicker::createPickingRenderPass() {
// the picking render pass

    AttachmentBuilder attachmentBuilder;

    ft::Attachment::pointer colorAttachment = attachmentBuilder
            .setDescriptionFormat(VK_FORMAT_R32_UINT)
            .setDescriptionSamples(VK_SAMPLE_COUNT_1_BIT)
            .setDescriptionLoadOp(VK_ATTACHMENT_LOAD_OP_CLEAR)
            .setDescriptionStoreOp(VK_ATTACHMENT_STORE_OP_STORE)
            .setDescriptionFinalLayout(VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
            .setReferenceImageLayout(VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
            .build();

    ft::Attachment::pointer depthAttachment = attachmentBuilder
            .setDescriptionFormat(_ftDevice->findDepthFormat())
            .setDescriptionSamples(VK_SAMPLE_COUNT_1_BIT)
            .setDescriptionLoadOp(VK_ATTACHMENT_LOAD_OP_CLEAR)
            .setDescriptionStoreOp(VK_ATTACHMENT_STORE_OP_DONT_CARE)
            .setDescriptionFinalLayout(VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
            .setReferenceImageLayout(VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
            .build();


    ft::SubPass::pointer subPass = std::make_shared<ft::SubPass>(VK_PIPELINE_BIND_POINT_GRAPHICS);
    ft::SubpassDependency::pointer subpassDependency = std::make_shared<ft::SubpassDependency>(VK_SUBPASS_EXTERNAL, 0);

    subpassDependency->setSrcStageMask(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT)
            .setSrcAccessMask(VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT)
            .setDstStageMask(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT)
            .setDstAccessMask(VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT);


    _ftRenderPass = std::make_shared<RenderPass>(_ftDevice);
    _ftRenderPass->addSubpass(subPass);
    _ftRenderPass->addSubpassDependency(subpassDependency);
    _ftRenderPass->addColorAttachmentToSubpass(colorAttachment);
    _ftRenderPass->setDepthStencilAttachmentToSubpass(depthAttachment);
    _ftRenderPass->create();
}

void ft::MousePicker::createPickingResources() {
    ImageBuilder imageBuilder;
    _ftColorImage = imageBuilder.setWidthHeight(_width, _height)
            .setMipLevel(1)
            .setSampleCount(VK_SAMPLE_COUNT_1_BIT)
            .setFormat(VK_FORMAT_R32_UINT)
            .setTiling(VK_IMAGE_TILING_OPTIMAL)
            .setUsageFlags(VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT)
            .setMemoryProperties(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
            .setAspectFlags(VK_IMAGE_ASPECT_COLOR_BIT)
            .build(_ftDevice);

    VkFormat depthFormat = _ftDevice->findDepthFormat();
    _ftDepthImage = imageBuilder.setWidthHeight(_width, _height)
            .setMipLevel(1)
            .setSampleCount(VK_SAMPLE_COUNT_1_BIT)
            .setFormat(depthFormat)
            .setTiling(VK_IMAGE_TILING_OPTIMAL)
            .setUsageFlags(VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)
            .setMemoryProperties(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
            .setAspectFlags(VK_IMAGE_ASPECT_DEPTH_BIT)
            .build(_ftDevice);

    Image::transitionImageLayout(_ftDevice, _ftDepthImage->getVKImage(), depthFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, 1);


    std::vector<VkImageView> attachments = {
            _ftColorImage->getVKImageView(),
            _ftDepthImage->getVKImageView(),
    };

    VkFramebufferCreateInfo framebufferCreateInfo{};
    framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferCreateInfo.renderPass = _ftRenderPass->getVKRenderPass();
    framebufferCreateInfo.attachmentCount = (uint32_t) attachments.size();
    framebufferCreateInfo.pAttachments = attachments.data();
    framebufferCreateInfo.width = _width;
    framebufferCreateInfo.height = _height;
    framebufferCreateInfo.layers = 1;

    if (vkCreateFramebuffer(_ftDevice->getVKDevice(), &framebufferCreateInfo, nullptr, &(_pickingFrameBuffer)) != VK_SUCCESS) {
        throw std::runtime_error("failed to create the picking framebuffer!");
    }


    // create the buffer for color data
    BufferBuilder bufferBuilder;

    _ftColorBuffer = bufferBuilder
            .setIsMapped(true)
            .setSize(_width * _height * 4)
            .setMappedOffset(0)
            .setMappedFlags(0)
            .setMemoryProperties(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)
            .setUsageFlags(VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT)
            .build(_ftDevice);
}

void ft::MousePicker::drawIDs(const ft::Scene::pointer &scene) {
    // wait for previous frame
    vkWaitForFences(_ftDevice->getVKDevice(), 1, &_fence, VK_TRUE, UINT64_MAX);
    vkResetFences(_ftDevice->getVKDevice(), 1, &_fence);

    // recording the command buffer
    vkResetCommandBuffer(_ftCommandBuffer->getVKCommandBuffer(), 0);

    // begin command buffer
    VkCommandBufferBeginInfo commandBufferBeginInfo{};
    commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    commandBufferBeginInfo.flags = 0;
    commandBufferBeginInfo.pInheritanceInfo = nullptr;

    if (vkBeginCommandBuffer(_ftCommandBuffer->getVKCommandBuffer(), &commandBufferBeginInfo) != VK_SUCCESS) {
        throw std::runtime_error("failed to begin recording command buffer!");
    }

    // starting render pass
    std::array<VkClearValue, 2> clearValues = {};
    clearValues[0].color =	{{0.0f, 0.0f, 0.0f, 0.0f}};
    clearValues[1].depthStencil = {1.0f, 0};


    VkRenderPassBeginInfo renderPassBeginInfo{};
    renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassBeginInfo.renderPass = _ftRenderPass->getVKRenderPass();
    renderPassBeginInfo.framebuffer = _pickingFrameBuffer;
    renderPassBeginInfo.renderArea.offset = {0, 0};
    renderPassBeginInfo.renderArea.extent = {_width, _height};
    renderPassBeginInfo.clearValueCount = (uint32_t) clearValues.size();
    renderPassBeginInfo.pClearValues = clearValues.data();

    vkCmdBeginRenderPass(_ftCommandBuffer->getVKCommandBuffer(), &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

    // bind the graphics pipeline
    vkCmdBindPipeline(_ftCommandBuffer->getVKCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, _ftPickingRdrSys->getGraphicsPipeline()->getVKPipeline());


    // bind the descriptor sets
    vkCmdBindDescriptorSets(_ftCommandBuffer->getVKCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS,
                            _ftPickingRdrSys->getGraphicsPipeline()->getVKPipelineLayout(), 0, 1,
                            &(_ftPickingRdrSys->getDescriptorSets()[0]->getVKDescriptorSet()),
                            0, nullptr);



    // set viewport and scissor
    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(_width);
    viewport.height = static_cast<float>(_height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(_ftCommandBuffer->getVKCommandBuffer(), 0, 1, &viewport);

    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = {_width, _height};
    vkCmdSetScissor(_ftCommandBuffer->getVKCommandBuffer(), 0, 1, &scissor);


    scene->drawPickObjs(_ftCommandBuffer, _ftPickingRdrSys->getGraphicsPipeline(), 0);


    // render pass end
    vkCmdEndRenderPass(_ftCommandBuffer->getVKCommandBuffer());

    // end command buffer
    if (vkEndCommandBuffer(_ftCommandBuffer->getVKCommandBuffer()) != VK_SUCCESS) {
        throw std::runtime_error("failed to record the picking command buffer!");
    }

    // submitting the command buffer
    auto cb = _ftCommandBuffer->getVKCommandBuffer();

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.waitSemaphoreCount = 0;
    submitInfo.pWaitSemaphores = nullptr;
    submitInfo.pWaitDstStageMask = nullptr;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &cb;
    submitInfo.signalSemaphoreCount = 0;
    submitInfo.pSignalSemaphores = nullptr;

    if (vkQueueSubmit(_ftDevice->getVKGraphicsQueue(), 1, &submitInfo, _fence) != VK_SUCCESS) {
        throw std::runtime_error("failed to submit a draw command buffer!");
    }

    Image::transitionImageLayout(_ftDevice, _ftColorImage->getVKImage(), VK_FORMAT_R32_UINT,
                                 VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, 1);
    _ftColorBuffer->copyFromImage(_ftColorImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);

    Image::transitionImageLayout(_ftDevice, _ftColorImage->getVKImage(), VK_FORMAT_R32_UINT,
                                 VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, 1);

}

void ft::MousePicker::createPickingSyncObjs() {
    VkFenceCreateInfo fenceCreateInfo{};
    fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT; // this is for the first call

    if (vkCreateFence(_ftDevice->getVKDevice(), &fenceCreateInfo, nullptr, &_fence) != VK_SUCCESS)
        throw std::runtime_error("failed to create the picking fence objects!");
}

void ft::MousePicker::updateResources(uint32_t width, uint32_t height) {
    // wait for previous frame
    vkWaitForFences(_ftDevice->getVKDevice(), 1, &_fence, VK_TRUE, UINT64_MAX);

    _width = width;
    _height = height;
    _ftColorImage.reset();
    _ftDepthImage.reset();
    vkDestroyFramebuffer(_ftDevice->getVKDevice(), _pickingFrameBuffer, nullptr);
    createPickingResources();
    _viewUpdated = true;
}



