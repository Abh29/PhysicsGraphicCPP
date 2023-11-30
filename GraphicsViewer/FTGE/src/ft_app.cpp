#include "../includes/ft_app.h"
#include "../includes/ft_callbacks.h"

ft::Application::Application() :
_ftWindow{W_WIDTH, W_HEIGHT, "applicationWindow", nullptr, ft::Callback::keyCallback}
{
	_validationLayers = {
			"VK_LAYER_KHRONOS_validation",
			"VK_LAYER_LUNARG_monitor"
	};

	_deviceExtensions = {
			VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};

	initApplication();
}

ft::Application::~Application() {
	delete _ftInstance;
}

void ft::Application::run() {

	while(!_ftWindow.shouldClose()) {
		_ftWindow.pollEvents();
	}
}

void ft::Application::initApplication() {

	VkApplicationInfo	applicationInfo{};
	applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	applicationInfo.pApplicationName = "Simple Application";
	applicationInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	applicationInfo.pEngineName = "No Engine";
	applicationInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	applicationInfo.apiVersion = VK_MAKE_API_VERSION(0, 1, 0, 0);
	applicationInfo.pNext = nullptr;

	_ftInstance = new Instance(applicationInfo, _validationLayers);
}
