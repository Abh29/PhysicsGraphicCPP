#ifndef FTGRAPHICS_FT_APP_H
#define FTGRAPHICS_FT_APP_H

#include "ft_headers.h"
#include "ft_defines.h"
#include "ft_window.h"
#include "ft_instance.h"
#include "ft_surface.h"
#include "ft_physicalDevice.h"
#include "ft_device.h"
#include "ft_image.h"
#include "ft_buffer.h"
#include "ft_command.h"
#include "ft_vertex.h"
#include "ft_renderPass.h"
#include "ft_attachment.h"
#include "ft_event.h"
#include "ft_gui.h"
#include "ft_scene.h"
#include "ft_pipeline.h"
#include "ft_sampler.h"
#include "ft_renderer.h"
#include "ft_descriptor.h"
#include "ft_rendering_systems.h"
#include "ft_material.h"
#include "ft_picker.h"

namespace ft {

	class Application {
	public:
		static constexpr uint32_t W_WIDTH = 800;
		static constexpr uint32_t W_HEIGHT = 600;

		Application();
		~Application();

		void run();

	private:
		void initEventListener();
		void initApplication();
		void createScene();
		static void printFPS();
		void updateScene(int key);
		void drawFrame();

		EventListener::pointer				_ftEventListener;
		Window::pointer						_ftWindow;
		Instance::pointer					_ftInstance;
		Surface::pointer					_ftSurface;
		PhysicalDevice::pointer				_ftPhysicalDevice;
		Device::pointer						_ftDevice;
		std::vector<const char*> 			_validationLayers;
		std::vector<const char*> 			_deviceExtensions;
		Gui::pointer 						_ftGui;
		Scene::pointer 						_ftScene;
		Renderer::pointer					_ftRenderer;
		SimpleRdrSys::pointer 				_ftSimpleRdrSys;
		TexturedRdrSys::pointer				_ftTexturedRdrSys;
        PickingRdrSys::pointer              _ftPickingRdrSys;
        MousePicker::pointer                _ftMousePicker;

		DescriptorPool::pointer 			_ftDescriptorPool;
		std::vector<DescriptorSet::pointer>	_ftDescriptorSets;
        MaterialPool::pointer               _ftMaterialPool;
		int									_topology = 0;
		uint32_t 							_currentFrame = 0;
	};

}

#endif //FTGRAPHICS_FT_APP_H
