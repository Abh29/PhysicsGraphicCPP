#include "../include.h"

ft::Instance::Instance(VkApplicationInfo &applicationInfo,
					   const std::vector<const char *> &validationLayers) :
					   _layers(validationLayers)
					   {
						   if (enableValidationLayers && !checkValidationLayerSupport()) {
							   throw std::runtime_error("validation layers requested, but not available!");
						   }

						   getRequiredExtensions();

						   VkDebugUtilsMessengerCreateInfoEXT debugCreateInfoExt{};
						   VkInstanceCreateInfo	createInfo{};
						   createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
						   createInfo.pApplicationInfo = &applicationInfo;
						   createInfo.enabledExtensionCount = static_cast<uint32_t>(_extensions.size());
						   createInfo.ppEnabledExtensionNames = _extensions.data();
						   if (enableValidationLayers) {
							   createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
							   createInfo.ppEnabledLayerNames = validationLayers.data();

							   populateDebugMessengerCreateInfo(debugCreateInfoExt);
							   createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT *) &debugCreateInfoExt;
						   } else {
							   createInfo.enabledLayerCount = 0;
							   createInfo.pNext = nullptr;
						   }

						   if ( vkCreateInstance(&createInfo, nullptr, &_instance) != VK_SUCCESS) {
							   throw std::runtime_error("failed to create an instance");
						   }

						   setupDebugMessenger();

					   }

ft::Instance::~Instance() {
	if (enableValidationLayers) {
		ft::Callback::DestroyDebugUtilsMessengerEXT(_instance, _debugMessenger, nullptr);
	}
	vkDestroyInstance(_instance, nullptr);
}

VkInstance ft::Instance::getVKInstance() const {return _instance;}

void ft::Instance::getRequiredExtensions() {
	uint32_t glfwExtensionCount = 0;
	const char** glfwExtensions;

	glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
	_extensions = {glfwExtensions, glfwExtensions + glfwExtensionCount};

	if (enableValidationLayers) {
		_extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	}
}

bool ft::Instance::checkValidationLayerSupport() {
	uint32_t layerCount;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

	std::vector<VkLayerProperties> availableLayers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

	for (const char* layerName : _layers) {
		bool layerFound = false;
		for (const auto& layerProperties: availableLayers) {
			if (std::strncmp(layerName, layerProperties.layerName, 255) == 0) {
				layerFound = true;
				break;
			}
		}
		if (!layerFound)
			return false;
	}
	return true;
}

void ft::Instance::populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfoExt) {
	createInfoExt = {};
	createInfoExt.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	createInfoExt.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
									VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
									VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	createInfoExt.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
								VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
								VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	createInfoExt.pfnUserCallback = ft::Callback::debugCallback;
	createInfoExt.pUserData = nullptr;
	createInfoExt.pNext = nullptr;
}

void ft::Instance::setupDebugMessenger() {
	if (!enableValidationLayers) return;

	VkDebugUtilsMessengerCreateInfoEXT createInfoExt{};
	populateDebugMessengerCreateInfo(createInfoExt);

	auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(_instance, "vkCreateDebugUtilsMessengerEXT");
	if (func != nullptr) {
		func(_instance, &createInfoExt, nullptr, &_debugMessenger);
	} else {
		throw std::runtime_error("failed to set up debug messenger!");
	}
}

