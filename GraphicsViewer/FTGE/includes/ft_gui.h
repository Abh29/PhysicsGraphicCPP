#ifndef FTGRAPHICS_FT_GUI_H
#define FTGRAPHICS_FT_GUI_H

#include "ft_headers.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"
#include "ft_command.h"
#include "ft_device.h"
#include "ft_window.h"
#include "ft_renderPass.h"
#include "ft_instance.h"


namespace ft {

    class Gui {

public:
	using pointer = std::shared_ptr<Gui>;

	Gui(Instance::pointer instance, PhysicalDevice::pointer physicalDevice,
		Device::pointer device, Window::pointer window,
		RenderPass::pointer renderPass, uint32_t imageCount);
	~Gui();

	void newFrame();
	void showDemo();
    void showGUI(uint32_t flags = 0, bool *p_open = nullptr);
	void render(CommandBuffer::pointer commandBuffer);



private:
    void showMetrics(bool *p_open);
    void showMainMenue();
    void showTitleBar();
    void showExampleMenuFile();

	Instance::pointer 						_ftInstance;
	PhysicalDevice::pointer 				_ftPhysicalDevice;
	Device::pointer 						_ftDevice;
	Window::pointer							_ftWindow;
	RenderPass::pointer						_ftRenderPass;
	VkDescriptorPool						_descriptorPool;

    // flags
    bool show_app_metrics = false;
    bool show_app_about = false;

};

}

#endif //FTGRAPHICS_FT_GUI_H
