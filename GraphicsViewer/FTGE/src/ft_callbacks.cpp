#include "../includes/ft_callbacks.h"

void ft::Callback::keyCallback(GLFWwindow *window, int key, int scancode,
                               int action, int mods) {
  if (action == GLFW_PRESS) {
    switch (key) {
    case GLFW_KEY_ESCAPE:
      glfwSetWindowShouldClose(window, true);
      break;
    default:
      std::cout << "key pressed " << key << ", " << scancode << ", " << mods
                << std::endl;
    }
  }
}

void ft::Callback::resizeCallback(GLFWwindow *window, int w, int h) {
  auto *ftw = reinterpret_cast<ft::Window *>(glfwGetWindowUserPointer(window));
  (void)ftw;
  (void)w;
  (void)h;
}

VkBool32 ft::Callback::debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
    void *pUserData) {
  (void)messageSeverity;
  (void)pUserData;
  switch (messageType) {
  case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
    std::cerr << "\033[32mvalidation layer [INFO]: " << pCallbackData->pMessage
              << "\033[0m" << std::endl;
    break;
  case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
    std::cerr << "\033[1;31mvalidation layer [WARN]: "
              << pCallbackData->pMessage << "\033[0m" << std::endl;
    break;
  default:
    std::cerr << "v\033[1;31malidation layer [ERROR]: "
              << pCallbackData->pMessage << "\033[0m" << std::endl;
  }

  return VK_FALSE;
}

void ft::Callback::DestroyDebugUtilsMessengerEXT(
    VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger,
    const VkAllocationCallbacks *pAllocator) {
  auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
      instance, "vkDestroyDebugUtilsMessengerEXT");
  if (func != nullptr) {
    func(instance, debugMessenger, pAllocator);
  }
}
