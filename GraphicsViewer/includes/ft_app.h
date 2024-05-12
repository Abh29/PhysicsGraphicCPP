#ifndef FTGRAPHICS_FT_APP_H
#define FTGRAPHICS_FT_APP_H

#include "ft_attachment.h"
#include "ft_buffer.h"
#include "ft_defines.h"
#include "ft_descriptor.h"
#include "ft_device.h"
#include "ft_event.h"
#include "ft_gui.h"
#include "ft_instance.h"
#include "ft_parser.h"
#include "ft_physicalDevice.h"
#include "ft_picker.h"
#include "ft_renderer.h"
#include "ft_rendering_systems.h"
#include "ft_scene.h"
#include "ft_surface.h"
#include "ft_texture.h"
#include "ft_threads.h"
#include "ft_window.h"

namespace ft {

class Application {
public:
  static constexpr uint32_t W_WIDTH = 800;
  static constexpr uint32_t W_HEIGHT = 600;

  Application();
  ~Application();

  void run();
  void setScenePath(const std::string &path);

private:
  void initEventListener();
  void initApplication();
  void createScene();
  static void printFPS();
  void updateScene(int key);
  void drawFrame();
  void checkEventQueue();
  void handleEven(Event::uniq_ptr e);

  EventListener::pointer _ftEventListener;
  Window::pointer _ftWindow;
  Instance::pointer _ftInstance;
  Surface::pointer _ftSurface;
  PhysicalDevice::pointer _ftPhysicalDevice;
  Device::pointer _ftDevice;
  std::vector<const char *> _validationLayers;
  std::vector<const char *> _deviceExtensions;
  Gui::pointer _ftGui;
  Scene::pointer _ftScene;
  Renderer::pointer _ftRenderer;
  SimpleRdrSys::pointer _ftSimpleRdrSys;
  OneTextureRdrSys::pointer _ftTexturedRdrSys;
  TwoTextureRdrSys::pointer _ft2TexturedRdrSys;
  SkyBoxRdrSys::pointer _ftSkyBoxRdrSys;
  PickingRdrSys::pointer _ftPickingRdrSys;
  OutlineRdrSys::pointer _ftOutlineRdrSys;
  PointRdrSys::pointer _ftPointRdrSys;
  LineRdrSys::pointer _ftLineRdrSys;
  NormDebugRdrSys::pointer _ftNormDebugRdrSys;
  MousePicker::pointer _ftMousePicker;
  ThreadPool::pointer _ftThreadPool;
  ft::JsonParser::pointer _ftJsonParser;

  DescriptorPool::pointer _ftDescriptorPool;
  std::vector<DescriptorSet::pointer> _ftDescriptorSets;
  TexturePool::pointer _ftMaterialPool;
  int _topology = 0;
  uint32_t _currentFrame = 0;
  std::string _scenePath;
  ft::GlobalState _ftGlobalState = {};
};

} // namespace ft

#endif // FTGRAPHICS_FT_APP_H
