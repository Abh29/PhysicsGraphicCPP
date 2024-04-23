#ifndef FTGRAPHICS_FT_INSTANCE_H
#define FTGRAPHICS_FT_INSTANCE_H

#include "ft_headers.h"

namespace ft {

#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

class Instance {

public:
  using pointer = std::shared_ptr<Instance>;

  Instance(VkApplicationInfo &applicationInfo,
           const std::vector<const char *> &validationLayers,
           const std::vector<const char *> &requiredExtensions);

  ~Instance();
  Instance(const Instance &other) = delete;
  Instance operator=(const Instance &other) = delete;

  VkInstance getVKInstance() const;

private:
  bool checkValidationLayerSupport();
  void populateDebugMessengerCreateInfo(
      VkDebugUtilsMessengerCreateInfoEXT &debugCreateInfoExt);
  void setupDebugMessenger();

  VkInstance _instance;
  VkDebugUtilsMessengerEXT _debugMessenger;
  std::vector<const char *> _layers;
  std::vector<const char *> _extensions;
};
} // namespace ft

#endif // FTGRAPHICS_FT_INSTANCE_H
