#ifndef FTGRAPHICS_FT_CALLBACKS_H
#define FTGRAPHICS_FT_CALLBACKS_H

#include "ft_headers.h"

namespace ft {
	struct Callback {

		static void resizeCallback(GLFWwindow *window, int w, int h);
		static void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods);

		static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
				VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
				VkDebugUtilsMessageTypeFlagsEXT messageType,
				const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
				void* pUserData);

		static void DestroyDebugUtilsMessengerEXT(
				VkInstance instance,
				VkDebugUtilsMessengerEXT debugMessenger,
				const VkAllocationCallbacks* pAllocator);
	};
}

#endif //FTGRAPHICS_FT_CALLBACKS_H
