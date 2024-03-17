#include "../includes/ft_app.h"
#include <glm/ext/matrix_transform.hpp>
#include <memory>
#include <vulkan/vulkan_core.h>

ft::Application::Application()
    : _ftEventListener(std::make_shared<ft::EventListener>()),
      _ftWindow{std::make_shared<Window>(W_WIDTH, W_HEIGHT, "applicationWindow",
                                         nullptr, _ftEventListener)},
      _ftThreadPool(std::make_shared<ft::ThreadPool>(ft::THREAD_POOL_SIZE)) {
  _validationLayers = {
      "VK_LAYER_KHRONOS_validation",
  };

  _deviceExtensions = {
      VK_KHR_SWAPCHAIN_EXTENSION_NAME,
  };

  initEventListener();
  initApplication();
  createScene();
}

ft::Application::~Application() = default;

void ft::Application::run() {

  while (!_ftWindow->shouldClose()) {
    _ftWindow->pollEvents();
    _ftGui->newFrame();
    _ftGui->showGUI();
    //		_ftGui->showDemo();
    drawFrame();
#ifdef SHOW_FRAME_RATE
    printFPS();
#endif
  }
  vkDeviceWaitIdle(_ftDevice->getVKDevice());
}

void ft::Application::initEventListener() {
  _ftEventListener->addCallbackForEventType(
      Event::EventType::KEYBOARD_EVENT, [&](ft::Event &ev) {
        auto &kev = dynamic_cast<KeyboardEvent &>(ev);
        auto data = kev.getData();
        if (std::any_cast<int>(data[2]) ==
                _ftWindow->ACTION(KeyActions::KEY_PRESS) ||
            std::any_cast<int>(data[2]) ==
                _ftWindow->ACTION(KeyActions::KEY_REPEAT))
          updateScene(std::any_cast<int>(data[0]));
      });

  _ftEventListener->addCallbackForEventType(
      Event::EventType::MOUSE_BUTTON, [&](ft::Event &ev) {
        auto &cev = dynamic_cast<CursorEvent &>(ev);
        auto data = cev.getData();
        if (std::any_cast<int>(data[1]) ==
            _ftWindow->ACTION(KeyActions::KEY_PRESS)) {
          auto x = std::any_cast<double>(data[3]);
          auto y = std::any_cast<double>(data[4]);
          if (y <= 30 || x >= (_ftRenderer->getSwapChain()->getWidth() - 200))
            return;
          uint32_t id =
              _ftMousePicker->pick(_ftScene, (uint32_t)x, (uint32_t)y);
          std::cout << "id is: " << id << std::endl;
          if (id && _ftScene->select(id)) {
            _ftMousePicker->notifyUpdatedView();
          }
        }
      });

  _ftEventListener->addCallbackForEventType(
      Event::EventType::MOUSE_SCROLL, [&](ft::Event &ev) {
        auto &sev = dynamic_cast<ScrollEvent &>(ev);
        auto data = sev.getData();
        auto yOff = std::any_cast<double>(data[1]);

        _ftScene->getCamera()->forward((float)yOff);
        _ftScene->updateCameraUBO();
        _ftMousePicker->notifyUpdatedView();
      });

  _ftEventListener->addCallbackForEventType(
      Event::EventType::SCREEN_RESIZE_EVENT, [&](ft::Event &ev) {
        auto &srev = dynamic_cast<ScreenResizeEvent &>(ev);
        auto data = srev.getData();
        auto width = std::any_cast<int>(data[0]);
        auto height = std::any_cast<int>(data[1]);
        _ftScene->getCamera()->updateAspect((float)width / (float)height);
        _ftScene->updateCameraUBO();
        _ftMousePicker->updateResources(
            _ftRenderer->getSwapChain()->getWidth(),
            _ftRenderer->getSwapChain()->getHeight());
      });
}

void ft::Application::initApplication() {

  VkApplicationInfo applicationInfo{};
  applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  applicationInfo.pApplicationName = "Simple Application";
  applicationInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
  applicationInfo.pEngineName = "No Engine";
  applicationInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
  applicationInfo.apiVersion = VK_MAKE_API_VERSION(0, 1, 3, 0);
  applicationInfo.pNext = nullptr;
  _ftInstance = std::make_shared<Instance>(applicationInfo, _validationLayers,
                                           _ftWindow->getRequiredExtensions());
  _ftSurface = std::make_shared<Surface>(_ftInstance, _ftWindow);
  _ftPhysicalDevice = std::make_shared<PhysicalDevice>(_ftInstance, _ftSurface,
                                                       _deviceExtensions);
  _ftDevice = std::make_shared<Device>(_ftPhysicalDevice, _validationLayers,
                                       _deviceExtensions);
  _ftRenderer = std::make_shared<ft::Renderer>(_ftWindow, _ftSurface,
                                               _ftPhysicalDevice, _ftDevice);
  _ftGui = std::make_shared<Gui>(_ftInstance, _ftPhysicalDevice, _ftDevice,
                                 _ftWindow, _ftRenderer->getRenderPass(),
                                 MAX_FRAMES_IN_FLIGHT);
  _ftMaterialPool = std::make_shared<ft::TexturePool>(_ftDevice);
  _ftDescriptorPool = std::make_shared<ft::DescriptorPool>(_ftDevice);
  _ftSimpleRdrSys = std::make_shared<ft::SimpleRdrSys>(_ftDevice, _ftRenderer,
                                                       _ftDescriptorPool);
  _ftSimpleRdrSys->populateUBODescriptors(_ftRenderer->getUniformBuffers());
  _ftTexturedRdrSys = std::make_shared<ft::OneTextureRdrSys>(
      _ftDevice, _ftRenderer, _ftDescriptorPool);
  _ft2TexturedRdrSys = std::make_shared<ft::TwoTextureRdrSys>(
      _ftDevice, _ftRenderer, _ftDescriptorPool);
  _ftSkyBoxRdrSys = std::make_shared<ft::SkyBoxRdrSys>(_ftDevice, _ftRenderer,
                                                       _ftDescriptorPool);
  _ftPickingRdrSys = std::make_shared<ft::PickingRdrSys>(_ftDevice, _ftRenderer,
                                                         _ftDescriptorPool);
  _ftPickingRdrSys->populateUBODescriptors(_ftRenderer->getUniformBuffers());
  _ftMousePicker = std::make_shared<ft::MousePicker>(
      _ftDevice, _ftRenderer->getSwapChain()->getWidth(),
      _ftRenderer->getSwapChain()->getHeight(), _ftPickingRdrSys);
  _ftOutlineRdrSys = std::make_shared<ft::OutlineRdrSys>(_ftDevice, _ftRenderer,
                                                         _ftDescriptorPool);
  _ftLineRdrSys = std::make_shared<ft::LineRdrSys>(_ftDevice, _ftRenderer,
                                                   _ftDescriptorPool);
  _ftPointRdrSys = std::make_shared<ft::PointRdrSys>(_ftDevice, _ftRenderer,
                                                     _ftDescriptorPool);
}

// TODO: replace this with a scene manager, read scene from disk
void ft::Application::createScene() {
  CameraBuilder cameraBuilder;
  Texture::pointer m;

  _ftScene =
      std::make_shared<Scene>(_ftDevice, _ftRenderer->getUniformBuffers());
  _ftScene->setMaterialPool(_ftMaterialPool);
  _ftScene->setCamera(cameraBuilder.setEyePosition({5, -1, 0})
                          .setTarget({1, -1, 0})
                          .setUpDirection({0, 1, 0})
                          .setFOV(120)
                          .setZNearFar(0.5f, 1000.0f)
                          .setAspect(_ftRenderer->getSwapChain()->getAspect())
                          .build());
  _ftScene->setGeneralLight({1.0f, 1.0f, 1.0f}, {0.0, 2.5f, 0.0f}, 0.2f);
  ft::InstanceData data{};
  (void)data;
  uint32_t id;
  (void)id;

  // Z
  data.model = glm::mat4(1.0f);
  data.model = glm::scale(data.model, {0.01f, 1.0f, 0.01f});
  data.color = {0.f, 0.0f, 0.9f};
  data.normalMatrix = glm::mat4(1.0f);
  _ftScene->addModelFromObj("models/axis.obj", data);

  // Y
  data.model = glm::rotate(glm::mat4(1), glm::radians(90.0f),
                           glm::vec3(1.0f, 0.0f, 0.0f));
  data.model = glm::scale(data.model, {0.01f, 1.0f, 0.01f});
  data.color = {0.0f, 0.9f, 0.0f};
  data.normalMatrix = glm::mat4(1.0f);
  _ftScene->addModelFromObj("models/axis.obj", data);

  // X
  data.model = glm::rotate(glm::mat4(1.0f), glm::radians(90.0f),
                           glm::vec3(0.0f, 0.0f, 1.0f));
  data.model = glm::scale(data.model, {0.01f, 1.0f, 0.01f});
  data.color = {0.9f, 0.0f, 0.0f};
  data.normalMatrix = glm::mat4(1.0f);
  _ftScene->addModelFromObj("models/axis.obj", data);

  // plane
  data.model = glm::mat4(1.0f);
  data.color = {0.95f, .95f, .95f};
  data.normalMatrix = glm::mat4(1.0f);
  data.model = glm::translate(data.model, {0, 1, 0});
  data.model = glm::scale(data.model, {300, 1, 300});
  _ftScene->addModelFromObj("models/plane.mtl.obj", data);
  //   _ftScene->addModelFromGltf("assets/models/plane.gltf", data);

  data.model = glm::mat4(1.0f);
  data.color = {0.95f, .9f, .5f};
  data.normalMatrix = glm::mat4(1.0f);
  data.model = glm::rotate(data.model, glm::radians(180.0f),
                           glm::vec3(0.0f, 1.0f, 1.0f));
  data.model = glm::translate(data.model, {0.0, 0.0, -5.7f});

  // auto cube = _ftScene->addModelFromGltf("assets/models/cube.gltf", data);
  auto venus = _ftScene->addModelFromGltf("assets/models/venus.gltf", data);
  venus->setFlags(venus->getID(), ft::MODEL_SELECTABLE_BIT);

  data.model = glm::mat4(1.0f);
  data.color = {0.95f, .95f, .95f};
  data.normalMatrix = glm::mat4(1.0f);
  data.model = glm::scale(data.model, {300, 300, 300});

  auto model = _ftScene->addCubeBox(
      "assets/models/sphere.gltf", "assets/textures/cubemap_space.ktx",
      _ftSkyBoxRdrSys->getDescriptorPool(),
      _ftSkyBoxRdrSys->getDescriptorSetLayout(), data);

  auto helmet = _ftScene->addSingleTexturedFromGltf(
      "assets/models/FlightHelmet/glTF/FlightHelmet.gltf",
      _ftTexturedRdrSys->getDescriptorPool(),
      _ftTexturedRdrSys->getDescriptorSetLayout());

  for (const auto &model : helmet) {
    glm::mat4 &mat = model->getRootModelMatrix();
    mat = glm::rotate(mat, glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    mat = glm::translate(mat, glm::vec3(10.0f, 0.45f, 0.0f));
  }

  auto castle = _ftScene->addDoubleTexturedFromGltf(
      "assets/models/sponza/sponza.gltf",
      _ft2TexturedRdrSys->getDescriptorPool(),
      _ft2TexturedRdrSys->getDescriptorSetLayout());

  for (const auto &model : castle) {
    glm::mat4 &mat = model->getRootModelMatrix();
    mat = glm::rotate(mat, glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    mat = glm::translate(mat, glm::vec3(0.0f, 0.45f, 0.0f));
    mat = glm::scale(mat, {2.0f, 2.0f, 2.0f});
  }

  // data.model = glm::mat4(1.0f);
  // data.color = {0.0f, 0.9f, 0.2f};
  // data.normalMatrix = glm::mat4(1.0f);
  // data.model = glm::rotate(data.model, glm::radians(90.0f), {1, 0, 0});
  // auto viking = _ftScene->addModelFromObj("models/viking_room.obj", data);

  // auto t = _ftMaterialPool->createTexture(
  //     "textures/viking_room.png", ft::Texture::FileType::FT_TEXTURE_PNG);
  // auto material = std::make_shared<Material>(_ftDevice);
  // material->addTexture(t);
  // material->createDescriptors(_ftTexturedRdrSys->getDescriptorPool(),
  //                             _ftTexturedRdrSys->getDescriptorSetLayout());
  // for (int i = 0; i < ft::MAX_FRAMES_IN_FLIGHT; ++i) {
  //   material->bindDescriptor(i, 0, 1);
  // }
  // _ftMaterialPool->addMaterial(material);
  // model->addMaterial(material);
  // model->unsetFlags(model->getID(), ft::MODEL_SIMPLE_BIT);
  // model->setFlags(model->getID(), ft::MODEL_HAS_COLOR_TEXTURE_BIT);
  //
  //     m = _ftTexturePool->createTexture("textures/viking_room.png",
  //     _ftRenderer->getSampler(),
  //                                            _ftTexturedRdrSys->getDescriptorPool(),
  //                                            _ftTexturedRdrSys->getDescriptorSetLayout(),
  //                                            Texture::FileType::FT_TEXTURE_UNDEFINED);
  //    _ftScene->addTextureToObject(id, m);

  //    data.model = glm::mat4(1.0f);
  //	data.color = {0.9f, 0.9f, 0.9f};
  //	data.normalMatrix = glm::mat4(1.0f);
  //    data.model = glm::translate(data.model, {0, 0, 3});
  //    data.model = glm::rotate(data.model, glm::radians(180.0f), {1,0,0});
  //    data.model = glm::scale(data.model, {0.005,0.005,0.005});
  //	id = _ftScene->addModelFromObj("models/car.obj", data);

  //    m = _ftTexturePool->createTexture("textures/car.png",
  //    _ftRenderer->getSampler(),
  //                                       _ftTexturedRdrSys->getDescriptorPool(),
  //                                       _ftTexturedRdrSys->getDescriptorSetLayout(),
  //                                       Texture::FileType::FT_TEXTURE_UNDEFINED);
  //    _ftScene->addTextureToObject(id, m);

  //    data.model = glm::mat4(1.0f);
  //	data.color = {0.9f, 0.9f, 0.9f};
  //	data.normalMatrix = glm::mat4(1.0f);
  //	data.model = glm::scale(data.model, {2, 2, 2});
  //	data.model = glm::rotate(data.model, glm::radians(180.0f), {1,0,0});
  //	id = _ftScene->addModelFromObj("models/big_terrain.obj", data);
  //
  //    auto m = _ftTexturePool->createTexture("textures/terrain.png",
  //    _ftRenderer->getSampler()); _ftScene->addTextureToObject(id, m);

  //	data.model = glm::mat4(1.0f);
  //	data.color = {0.9f, 0.9f, 0.9f};
  //	data.normalMatrix = glm::mat4(1.0f);
  ////	data.model = glm::scale(data.model, {0.1, 0.1, 0.1});
  //	data.model = glm::rotate(data.model, glm::radians(180.0f), {1,0,0});
  //	id = _ftScene->addModelFromObj("models/mountain.obj", data);
  //
  //    m = _ftTexturePool->createTexture("textures/mountain.jpg",
  //    _ftRenderer->getSampler(), _ftTexturedRdrSys->getDescriptorPool(),
  //    _ftTexturedRdrSys->getDescriptorSetLayout());
  //    _ftScene->addTextureToObject(id, m);

  //	data.model = glm::rotate(glm::mat4(1.0f), glm::radians(0.0f),
  // glm::vec3(0.0f, 0.0f, 1.0f)); 	data.model = glm::translate(data.model,
  // glm::vec3{1.0f, -2.0f, 0.0f}); 	data.color = {0.5f, 0.95f, 0.2f};
  // id = _ftScene->addObjectCopyToTheScene(id, data);

  //
  //	data.model = glm::rotate(glm::mat4(1.0f), glm::radians(0.0f),
  // glm::vec3(0.0f, 0.0f, 1.0f)); 	data.model = glm::translate(data.model,
  // glm::vec3{0.0f, 3.0f, 0.0f}); 	data.color = {0.7f, 0.2f, 0.4f};
  // id = _ftScene->addObjectCopyToTheScene(id, data);
  //
  //	id = _ftScene->addModelFromObj("models/smooth_vase.obj", {
  //			glm::rotate(glm::mat4(1.0f), glm::radians(90.0f),
  // glm::vec3(0.0f, 0.0f, 1.0f)), 			glm::mat4(1.0f),
  // {0.95f, 0.2f, 0.0f}
  //	});

  //	data.model = glm::mat4(1.0f);
  //	data.color = {0.95f, 0.2f, 0.0f};
  //	data.normalMatrix = glm::mat4(1.0f);
  //	id = _ftScene->addModelFromObj("models/sphere.mtl.obj", data);
  //
  //	data.model = glm::rotate(glm::mat4(1.0f), glm::radians(0.0f),
  // glm::vec3(0.0f, 0.0f, 1.0f)); 	data.model = glm::translate(data.model,
  // glm::vec3{1.0f, -2.0f, 0.0f}); 	data.color = {0.5f, 0.95f, 0.2f};
  // id = _ftScene->addObjectCopyToTheScene(id, data);

  (void)id;
}

void ft::Application::printFPS() {
  static auto oldTime = std::chrono::high_resolution_clock::now();
  static int fps;

  fps++;
  if (std::chrono::duration_cast<std::chrono::seconds>(
          std::chrono::high_resolution_clock::now() - oldTime) >=
      std::chrono::seconds{1}) {
    oldTime = std::chrono::high_resolution_clock::now();
    std::cout << "FPS: " << fps << std::endl;
    fps = 0;
  }
}

// draw a frame
void ft::Application::drawFrame() {
  CommandBuffer::pointer commandBuffer;
  uint32_t index;
  std::tie(index, commandBuffer) = _ftRenderer->beginFrame();
  if (!commandBuffer)
    return;

  // begin the render pass
  _ftRenderer->beginRenderPass(commandBuffer, index);

  _ftScene->drawSimpleObjs(commandBuffer,
                           _ftSimpleRdrSys->getGraphicsPipeline(),
                           _ftSimpleRdrSys, _currentFrame);

  _ftScene->drawPointsTopology(commandBuffer, _ftSimpleRdrSys, _ftPointRdrSys,
                               _currentFrame);
  _ftScene->drawLinesTopology(commandBuffer, _ftSimpleRdrSys, _ftLineRdrSys,
                              _currentFrame);

  _ftScene->drawTexturedObjs(commandBuffer,
                             _ftTexturedRdrSys->getGraphicsPipeline(),
                             _ftTexturedRdrSys, _currentFrame);

  _ftScene->draw2TexturedObjs(commandBuffer,
                              _ft2TexturedRdrSys->getGraphicsPipeline(),
                              _ft2TexturedRdrSys, _currentFrame);

  _ftScene->drawSkyBox(commandBuffer, _ftSkyBoxRdrSys->getGraphicsPipeline(),
                       _ftSkyBoxRdrSys, _currentFrame);

  _ftScene->drawOulines(commandBuffer, _ftSimpleRdrSys, _ftOutlineRdrSys,
                        _currentFrame);

  // gui
  _ftGui->render(commandBuffer);

  // end the render pass
  _ftRenderer->endRenderPass(commandBuffer, index);
  // end the frame
  _ftRenderer->endFrame(commandBuffer, index);
}

void ft::Application::updateScene(int key) {
  if (key == _ftWindow->KEY(KeyboardKeys::KEY_UP)) {
    _ftScene->getCamera()->vRotate(-10.0f);
  } else if (key == _ftWindow->KEY(KeyboardKeys::KEY_DOWN)) {
    _ftScene->getCamera()->vRotate(10.0f);
  } else if (key == _ftWindow->KEY(KeyboardKeys::KEY_RIGHT)) {
    _ftScene->getCamera()->hRotate(10.0f);
  } else if (key == _ftWindow->KEY(KeyboardKeys::KEY_LEFT)) {
    _ftScene->getCamera()->hRotate(-10.0f);
  } else if (key == _ftWindow->KEY(KeyboardKeys::KEY_R)) {
    _ftScene->getCamera()->hardSet({5, -1, 0}, {1, -1, 0}, {0, 1, 0});
  } else if (key == _ftWindow->KEY(KeyboardKeys::KEY_W)) {
    _ftScene->getCamera()->forward(0.5f);
  } else if (key == _ftWindow->KEY(KeyboardKeys::KEY_Q)) {
    _ftScene->getCamera()->translateUp(-0.5f);
  } else if (key == _ftWindow->KEY(KeyboardKeys::KEY_E)) {
    _ftScene->getCamera()->translateUp(0.5f);
  } else if (key == _ftWindow->KEY(KeyboardKeys::KEY_A)) {
    _ftScene->getCamera()->translateSide(-0.5f);
  } else if (key == _ftWindow->KEY(KeyboardKeys::KEY_S)) {
    _ftScene->getCamera()->forward(-0.5f);
  } else if (key == _ftWindow->KEY(KeyboardKeys::KEY_D)) {
    _ftScene->getCamera()->translateSide(0.5f);
  } else if (key == _ftWindow->KEY(KeyboardKeys::KEY_KP_7)) {
    _ftScene->getCamera()->rotateWorldX(10.0f);
  } else if (key == _ftWindow->KEY(KeyboardKeys::KEY_KP_9)) {
    _ftScene->getCamera()->rotateWorldX(-10.0f);
  } else if (key == _ftWindow->KEY(KeyboardKeys::KEY_KP_4)) {
    _ftScene->getCamera()->rotateWorldY(10.0f);
  } else if (key == _ftWindow->KEY(KeyboardKeys::KEY_KP_6)) {
    _ftScene->getCamera()->rotateWorldY(-10.0f);
  } else if (key == _ftWindow->KEY(KeyboardKeys::KEY_KP_1)) {
    _ftScene->getCamera()->rotateWorldZ(10.0f);
  } else if (key == _ftWindow->KEY(KeyboardKeys::KEY_KP_3)) {
    _ftScene->getCamera()->rotateWorldZ(-10.0f);
  } else if (key == _ftWindow->KEY(KeyboardKeys::KEY_Z)) {
    std::cout << "unselect all ... " << std::endl;
    _ftScene->unselectAll();
  } else if (key == _ftWindow->KEY(KeyboardKeys::KEY_H)) {
    _ftScene->hideSelected();
  } else if (key == _ftWindow->KEY(KeyboardKeys::KEY_T)) {
    _ftScene->unhideAll();
  } else if (key == _ftWindow->KEY(KeyboardKeys::KEY_P)) {
    auto mp = _ftScene->getSelectedModel();
    if (mp) {
      mp->toggleFlags(mp->ID(), ft::MODEL_POINT_BIT);
      std::cout << "drawing points " << mp->hasFlag(ft::MODEL_POINT_BIT)
                << std::endl;
    }

  } else if (key == _ftWindow->KEY(KeyboardKeys::KEY_L)) {
    auto mp = _ftScene->getSelectedModel();
    if (mp) {
      mp->toggleFlags(mp->ID(), ft::MODEL_LINE_BIT);
      std::cout << "drawing lines " << mp->hasFlag(ft::MODEL_LINE_BIT)
                << std::endl;
    }
  }

  _ftScene->updateCameraUBO();
  _ftMousePicker->notifyUpdatedView();
}
