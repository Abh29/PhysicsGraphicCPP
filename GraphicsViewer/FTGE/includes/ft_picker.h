#ifndef FTGRAPHICS_FT_PICKER_H
#define FTGRAPHICS_FT_PICKER_H

#include "ft_headers.h"
#include "ft_defines.h"
#include "ft_device.h"
#include "ft_swapChain.h"
#include "ft_image.h"
#include "ft_buffer.h"
#include "ft_attachment.h"
#include "ft_scene.h"
#include "ft_rendering_systems.h"

namespace ft {

 class MousePicker {
 public:
     using pointer = std::shared_ptr<MousePicker>;

     MousePicker(Device::pointer device, uint32_t width, uint32_t height, PickingRdrSys::pointer rdrSys);
     ~MousePicker();

     [[nodiscard]] uint32_t pick(Scene::pointer &scene, uint32_t x, uint32_t y);
     void notifyUpdatedView();
     void updateResources(uint32_t width, uint32_t height);
     [[nodiscard]] VkFence getVkFence() const;

 private:
     void createPickingResources();
     void createPickingRenderPass();
     void createPickingSyncObjs();
     void drawIDs(Scene::pointer &scene);


     Device::pointer                    _ftDevice;
     uint32_t                           _width;
     uint32_t                           _height;
     bool                               _viewUpdated = true;
     VkFramebuffer 						_pickingFrameBuffer = VK_NULL_HANDLE;
     RenderPass::pointer 				_ftRenderPass;
     CommandBuffer::pointer             _ftCommandBuffer;
     Image::pointer 					_ftColorImage;
     Image::pointer 					_ftDepthImage;
     Buffer::pointer                    _ftColorBuffer;
     VkFence                            _fence;
     PickingRdrSys::pointer             _ftPickingRdrSys;
 };
}


#endif //FTGRAPHICS_FT_PICKER_H
