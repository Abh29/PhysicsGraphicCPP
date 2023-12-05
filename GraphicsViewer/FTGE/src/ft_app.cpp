#include "../includes/ft_app.h"
#include "../includes/ft_callbacks.h"

ft::Application::Application() :
_ftWindow{std::make_shared<Window>(W_WIDTH, W_HEIGHT, "applicationWindow", nullptr, ft::Callback::keyCallback)}
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
	cleanup();
}

void ft::Application::run() {

	while(!_ftWindow->shouldClose()) {
		_ftWindow->pollEvents();
		drawFrame();
	}
	vkDeviceWaitIdle(_ftDevice->getVKDevice());
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

	_ftInstance = std::make_shared<Instance>(applicationInfo, _validationLayers, _ftWindow->getRequiredExtensions());
	_ftSurface = std::make_shared<Surface>(_ftInstance, _ftWindow);
	_ftPhysicalDevice = std::make_shared<PhysicalDevice>(_ftInstance, _ftSurface, _deviceExtensions);
	_ftDevice = std::make_shared<Device>(_ftPhysicalDevice, _validationLayers, _deviceExtensions);
	_ftSwapChain = std::make_shared<SwapChain>(_ftPhysicalDevice, _ftDevice, _ftSurface,
											   _ftWindow->queryCurrentWidthHeight().first,
											   _ftWindow->queryCurrentWidthHeight().second);
	_ftCommandPool = std::make_shared<CommandPool>(_ftDevice);
	for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
		_ftCommandBuffers.push_back(std::make_shared<CommandBuffer>(
				_ftDevice, _ftCommandPool));
	}

	_ftImageBuilder = std::make_shared<ImageBuilder>();
	_ftBufferBuilder = std::make_shared<BufferBuilder>();

	createRenderPass();
	createDescriptorSetLayout();
	createGraphicsPipeline();
	createColorResources();
	createDepthResources();
	createFramebuffers();
	createTextureImage();
	createTextureSampler();
	loadModel();
	createVertexBuffer();
	createIndexBuffer();
	createUniformBuffers();
	createDescriptorPool();
	createDescriptorSets();
	createSyncObjects();
}
