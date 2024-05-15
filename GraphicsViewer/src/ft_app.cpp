#include "../includes/ft_app.h"
#include <chrono>
#include <exception>
#include <functional>
#include <glm/ext/matrix_transform.hpp>
#include <glm/fwd.hpp>
#include <glm/geometric.hpp>
#include <glm/gtc/quaternion.hpp>
#include <ios>
#include <memory>
#include <thread>
#include <vulkan/vulkan_core.h>

#define SHOW_AXIS 1
#define SHOW_PLANE 0
#define SHOW_HELMET 0
#define SHOW_SPONZA 0
#define SHOW_VENUS 0
#define SHOW_VIKINGS 0
#define SHOW_SKYBOX 0
#define SHOW_CAR 0
#define SHOW_TERRAIN 0
#define SHOW_ARROW 0
#define SHOW_UNIT_BOX 0
#define SHOW_GIZMO 0

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

  float duration = 0.1f;
  while (!_ftWindow->shouldClose()) {
    _ftWindow->pollEvents();
    _ftGui->newFrame();
    _ftGui->showGUI();
    _ftGui->showDemo();
    duration = 1.0f / _ftGui->getFramerate();
    if (_play) {
      _ftPhysicsApplication->update(duration);
      _ftScene->updateSceneObjects(duration, _ftThreadPool);
    }
    drawFrame();
    checkEventQueue();
#ifdef SHOW_FRAME_RATE
    printFPS();
#endif
  }
  vkDeviceWaitIdle(_ftDevice->getVKDevice());
}

void ft::Application::initEventListener() {
  _ftEventListener->addCallbackForEventType(
      Event::EventType::KEYBOARD_EVENT, [&](ft::Event &ev) {
        if (_ftGui->isKeyCaptured())
          return;

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
        if (_ftGui->isMouseCaptured())
          return;

        auto &cev = dynamic_cast<CursorEvent &>(ev);
        auto data = cev.getData();
        auto x = std::any_cast<double>(data[3]);
        auto y = std::any_cast<double>(data[4]);
        if (y <= 30 || x >= (_ftRenderer->getSwapChain()->getWidth() - 200))
          return;
        uint32_t id = _ftMousePicker->pick(_ftScene, (uint32_t)x, (uint32_t)y);
        std::cout << "id is: " << id << std::endl;
        if (id && _ftScene->select(id)) {
          _ftMousePicker->notifyUpdatedView();
        }
        if (_ftScene->hasGizmo()) {
          _ftScene->getGizmo()->unselect();
          _ftScene->getGizmo()->select(id);
        }
      });

  _ftEventListener->addCallbackForEventType(
      Event::EventType::MOUSE_SCROLL, [&](ft::Event &ev) {
        if (_ftGui->isGuiHovered())
          return;

        auto &sev = dynamic_cast<ScrollEvent &>(ev);
        auto data = sev.getData();
        auto yOff = std::any_cast<double>(data[1]);

        if (_ftScene->hasGizmo() && _ftScene->getGizmo()->isSelected()) {
          auto e = _ftScene->getGizmo()->getSelected();
          auto m = _ftScene->getSelectedModel();
          if (m == nullptr)
            return;
          switch (e) {
          case ft::Gizmo::Elements::X_ARROW:
            m->translate(glm::vec3(0.0f, 0.0f, -0.1f * yOff),
                         _ftScene->isGlobalGizmo());
            break;
          case ft::Gizmo::Elements::Y_ARROW:
            m->translate({0.1f * yOff, 0.0f, 0.00f}, _ftScene->isGlobalGizmo());
            break;
          case ft::Gizmo::Elements::Z_ARROW:
            m->translate({0.0f, 0.1f * yOff, 0.0f}, _ftScene->isGlobalGizmo());
            break;
          case ft::Gizmo::Elements::X_RING:
            m->rotate(glm::vec3(1.0f, 0.0f, 0.0f), 10.0f * yOff,
                      _ftScene->isGlobalGizmo());
            break;
          case ft::Gizmo::Elements::Y_RING:
            m->rotate(glm::vec3(0.0f, 1.0f, 0.0f), 10.0f * yOff,
                      _ftScene->isGlobalGizmo());
            break;
          case ft::Gizmo::Elements::Z_RING:
            m->rotate(glm::vec3(0.0f, 0.0f, 1.0f), 10.0f * yOff,
                      _ftScene->isGlobalGizmo());
            break;
          default:
            break;
          }

          // todo: test this
          if (m->hasFlag(ft::MODEL_HAS_RIGID_BODY_BIT)) {
            auto objs = _ftScene->getObjects();
            SceneObject::pointer obj = nullptr;
            for (auto &i : objs) {
              if (i->getModel().get() == m) {
                obj = i;
                break;
              }
            }

            if (obj.get()) {
              RigidBallComponent::raw_ptr rball = nullptr;
              RigidBoxComponent::raw_ptr rbox = nullptr;

              rball = obj->getComponent<RigidBallComponent>();
              if (rball) {
                rball->backwardUpdate(1.0f);
              }

              rbox = obj->getComponent<RigidBoxComponent>();
              if (rbox) {
                rbox->backwardUpdate(1.0f);
              }
            }
          }

        } else {
          _ftScene->getCamera()->forward((float)yOff * 3);
        }
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

  _ftEventListener->addCallbackForEventType(
      Event::EventType::MOUSE_BUTTON_DRAG, [&](ft::Event &ev) {
        if (_ftGui->isMouseCaptured())
          return;

        auto &srev = dynamic_cast<CursorDragEvent &>(ev);
        auto data = srev.getData();
        auto x = std::any_cast<double>(data[1]);
        auto y = std::any_cast<double>(data[2]);

        if (y <= 30 || x >= (_ftRenderer->getSwapChain()->getWidth() - 200))
          return;

        if (_ftScene->hasGizmo() && _ftScene->getGizmo()->isSelected()) {
          auto e = _ftScene->getGizmo()->getSelected();
          auto m = _ftScene->getSelectedModel();
          if (m == nullptr)
            return;
          switch (e) {
          case ft::Gizmo::Elements::X_ARROW:
            m->translate(glm::vec3(0.0f, 0.0f, 0.1f));
            break;
          case ft::Gizmo::Elements::Y_ARROW:
            m->translate({0.2f, 0.0f, 0.00f});
            break;
          case ft::Gizmo::Elements::Z_ARROW:
            m->translate({0.0f, -0.05f, 0.0f});
            break;
          case ft::Gizmo::Elements::X_RING:
            m->rotate(glm::vec3(1.0f, 0.0f, 0.0f), 10.0f);
            break;
          case ft::Gizmo::Elements::Y_RING:
            m->rotate(glm::vec3(0.0f, 0.0f, 1.0f), 10.0f);
            break;
          case ft::Gizmo::Elements::Z_RING:
            m->rotate(glm::vec3(0.0f, 1.0f, 0.0f), 10.0f);
            break;
          default:
            break;
          }

          // todo: test this
          if (m->hasFlag(ft::MODEL_HAS_RIGID_BODY_BIT)) {
            auto objs = _ftScene->getObjects();
            SceneObject::pointer obj = nullptr;
            for (auto &i : objs) {
              if (i->getModel().get() == m) {
                obj = i;
                break;
              }
            }

            if (obj.get()) {
              RigidBallComponent::raw_ptr rball = nullptr;
              RigidBoxComponent::raw_ptr rbox = nullptr;

              rball = obj->getComponent<RigidBallComponent>();
              if (rball) {
                rball->backwardUpdate(1.0f);
              }

              rbox = obj->getComponent<RigidBoxComponent>();
              if (rbox) {
                rbox->backwardUpdate(1.0f);
              }
            }
          }

          _ftScene->updateCameraUBO();
          _ftMousePicker->notifyUpdatedView();
        }
      });

  _ftEventListener->addCallbackForEventType(
      Event::EventType::MOUSE_BUTTON_DRAG_RELEASE, [&](ft::Event &ev) {
        (void)ev;
        if (_ftScene->hasGizmo())
          _ftScene->getGizmo()->unselect();
      });

  // gui menue events
  // file
  _ftEventListener->addCallbackForEventType(Event::EventType::Menue_File_NEW,
                                            [&](ft::Event &ev) { (void)ev; });

  _ftEventListener->addCallbackForEventType(Event::EventType::Menue_File_OPEN,
                                            [&](ft::Event &ev) { (void)ev; });

  _ftEventListener->addCallbackForEventType(
      Event::EventType::Menue_File_SAVE, [&](ft::Event &ev) {
        (void)ev;
        _ftJsonParser->saveSceneToFile(_ftScene, "assets/ft_scene.json");
      });

  _ftEventListener->addCallbackForEventType(
      Event::EventType::Menue_File_RELOAD, [&](ft::Event &ev) {
        (void)ev;
        std::cout << "reload scene from " << _scenePath << std::endl;

        vkDeviceWaitIdle(_ftDevice->getVKDevice());
        _ftScene.reset();
        createScene();
        if (!_scenePath.empty()) {
          _ftJsonParser->parseSceneFile(
              _ftScene, _scenePath, _ftRenderer->getSwapChain()->getAspect());
          _ftScene->updateCameraUBO();
          _ftMousePicker->notifyUpdatedView();
        } else {
          std::cout << "scene file empty!" << std::endl;
        }
      });

  _ftEventListener->addCallbackForEventType(Event::EventType::Menue_File_SAVEAS,
                                            [&](ft::Event &ev) { (void)ev; });

  _ftEventListener->addCallbackForEventType(Event::EventType::Menue_File_QUIT,
                                            [&](ft::Event &ev) { (void)ev; });

  // insert
  _ftEventListener->addCallbackForEventType(
      Event::EventType::Menue_Insert_MODEL, [&](ft::Event &ev) { (void)ev; });

  _ftEventListener->addCallbackForEventType(
      Event::EventType::Menue_Insert_SKYBOX, [&](ft::Event &ev) { (void)ev; });

  _ftEventListener->addCallbackForEventType(
      Event::EventType::Menue_Insert_UBOX, [&](ft::Event &ev) {
        (void)ev;
        ft::ObjectState data{};
        try {

          auto unit_box =
              _ftScene->addModelFromObj("misk/models/cube.mtl.obj", data);
          unit_box->getModel()->setFlags(unit_box->getModel()->getID(),
                                         ft::MODEL_SELECTABLE_BIT |
                                             ft::MODEL_SIMPLE_BIT);

          RigidBox::pointer box = std::make_shared<RigidBox>();
          box->setState({}, {1.0f, 0.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 1.0f},
                        {0.0f, 1.0f, 0.0f});

          _ftPhysicsApplication->addRigidBox(box);
          unit_box->addComponent<RigidBoxComponent>(unit_box->getModel(), box);
          unit_box->getModel()->setFlags(unit_box->getModel()->getID(),
                                         ft::MODEL_HAS_RIGID_BODY_BIT);

        } catch (std::exception &e) {
          std::cerr
              << "cant load model, make sure it exists in the assets folder !"
              << std::endl;
        }
      });

  _ftEventListener->addCallbackForEventType(
      Event::EventType::Menue_Insert_USPHERE, [&](ft::Event &ev) {
        (void)ev;

        ft::ObjectState data{};
        try {

          auto unit_sphere =
              _ftScene->addModelFromObj("misk/models/sphere.mtl.obj", data);
          unit_sphere->getModel()->setFlags(unit_sphere->getModel()->getID(),
                                            ft::MODEL_SIMPLE_BIT |
                                                ft::MODEL_SELECTABLE_BIT);

          RigidBall::pointer ball = std::make_shared<RigidBall>();
          ball->setState(
              {}, glm::quat_cast(unit_sphere->getModel()->getState().rotation),
              1.0f, {0.0f, 1.0f, 0.0f});

          _ftPhysicsApplication->addRigidBall(ball);

          unit_sphere->addComponent<RigidBallComponent>(unit_sphere->getModel(),
                                                        ball);
          unit_sphere->getModel()->setFlags(unit_sphere->getModel()->getID(),
                                            ft::MODEL_HAS_RIGID_BODY_BIT);

        } catch (std::exception &e) {
          std::cerr
              << "cant load model, make sure it exists in the assets folder !"
              << std::endl;
        }
      });

  // edit
  _ftEventListener->addCallbackForEventType(
      Event::EventType::Menue_Edit_UNSELECTALL, [&](ft::Event &ev) {
        (void)ev;
        _ftScene->unselectAll();
      });

  _ftEventListener->addCallbackForEventType(
      Event::EventType::Menue_Edit_UNHIDEALL, [&](ft::Event &ev) {
        (void)ev;
        _ftScene->unhideAll();
      });

  _ftEventListener->addCallbackForEventType(
      Event::EventType::Menue_Edit_DEFAULTTOPO, [&](ft::Event &ev) {
        (void)ev;
        auto s = _ftScene->getSelectedModel();
        if (s) {
          s->unsetFlags(s->getID(), ft::MODEL_LINE_BIT | ft::MODEL_POINT_BIT);
        }
      });

  _ftEventListener->addCallbackForEventType(
      Event::EventType::Menue_Edit_LINETOPO, [&](ft::Event &ev) {
        (void)ev;
        auto s = _ftScene->getSelectedModel();
        if (s) {
          s->unsetFlags(s->getID(), ft::MODEL_POINT_BIT);
          s->setFlags(s->getID(), ft::MODEL_LINE_BIT);
        }
      });

  _ftEventListener->addCallbackForEventType(
      Event::EventType::Menue_Edit_POINTTOP, [&](ft::Event &ev) {
        (void)ev;
        auto s = _ftScene->getSelectedModel();
        if (s) {
          s->unsetFlags(s->getID(), ft::MODEL_LINE_BIT);
          s->setFlags(s->getID(), ft::MODEL_POINT_BIT);
        }
      });

  _ftEventListener->addCallbackForEventType(
      Event::EventType::Menue_Edit_RECALCNORM, [&](ft::Event &ev) {
        (void)ev;
        _ftScene->calculateNormals();
      });

  _ftEventListener->addCallbackForEventType(
      Event::EventType::Menue_Edit_SHOWNORM, [&](ft::Event &ev) {
        (void)ev;
        _ftScene->toggleNormalDebug();
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
                                 MAX_FRAMES_IN_FLIGHT, _ftGlobalState);
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
  _ftNormDebugRdrSys = std::make_shared<ft::NormDebugRdrSys>(
      _ftDevice, _ftRenderer, _ftDescriptorPool);
  _ftJsonParser = std::make_shared<ft::JsonParser>(
      _ftDevice, _ftMaterialPool, _ftThreadPool, _ftTexturedRdrSys,
      _ft2TexturedRdrSys, _ftSkyBoxRdrSys);

  _ftPhysicsApplication = std::make_shared<ft::SimpleRigidApplication>(512);
}

// TODO: replace this with a scene manager, read scene from disk
void ft::Application::createScene() {
  // CameraBuilder cameraBuilder;
  Texture::pointer m;

  _ftScene =
      std::make_shared<Scene>(_ftDevice, _ftRenderer->getUniformBuffers());
  _ftScene->setMaterialPool(_ftMaterialPool);
  // _ftScene->setCamera(cameraBuilder.setEyePosition({5, -1, 0})
  //                         .setTarget({1, -1, 0})
  //                         .setUpDirection({0, 1, 0})
  //                         .setFOV(120)
  //                         .setZNearFar(0.5f, 1000.0f)
  //                         .setAspect(_ftRenderer->getSwapChain()->getAspect())
  //                         .build());
  // _ftScene->setGeneralLight({1.0f, 1.0f, 1.0f}, {0.0, 2.5f, 0.0f}, 0.2f);
  auto gizmo = _ftScene->loadGizmo("assets/models/gizmo.gltf");
  ft::ObjectState data{};
  (void)data;
  uint32_t id;
  (void)id;

#if SHOW_AXIS

  // Z
  data.scaling = glm::scale(glm::mat4(1.0f), {0.01f, 1.0f, 0.01f});
  data.color = {0.f, 0.0f, 0.9f};
  data.loadOptions = LOAD_OPTION_NO_SAVE;
  _ftScene->addModelFromObj("misk/models/axis.obj", data);

  // Y
  data.rotation = glm::rotate(glm::mat4(1.0f), glm::radians(90.0f),
                              glm::vec3(1.0f, 0.0f, 0.0f));
  data.scaling = glm::scale(glm::mat4(1.0f), {0.01f, 1.0f, 0.01f});
  data.color = {0.0f, 0.9f, 0.0f};
  _ftScene->addModelFromObj("misk/models/axis.obj", data);

  // X
  data.rotation = glm::rotate(glm::mat4(1.0f), glm::radians(90.0f),
                              glm::vec3(0.0f, 0.0f, 1.0f));
  data.scaling = glm::scale(glm::mat4(1.0f), {0.01f, 1.0f, 0.01f});
  data.color = {0.9f, 0.0f, 0.0f};
  _ftScene->addModelFromObj("misk/models/axis.obj", data);
  data.loadOptions = 0;
#endif
#if SHOW_PLANE
  // plane
  data.color = glm::vec3{1.0f, 1.0f, 1.0f};
  data.scaling = glm::scale(glm::mat4(1.0f), {300, 3, 300});
  data.rotation = glm::rotate(glm::mat4(1.0f), glm::radians(90.0f),
                              glm::vec3(0.0f, 1.0f, 0.0f));
  auto plane = _ftScene->addModelFromObj("assets/models/plane.mtl.obj", data);
#endif
#if SHOW_SKYBOX

  // skybox
  data.color = {0.95f, .95f, .95f};
  data.scaling = glm::scale(glm::mat4(1.0f), {300, 300, 300});
  auto skybox = _ftScene->addCubeBox(
      "assets/models/sphere.gltf", "assets/textures/cubemap_space.ktx",
      _ftSkyBoxRdrSys->getDescriptorPool(),
      _ftSkyBoxRdrSys->getDescriptorSetLayout(), data);
#endif
#if SHOW_HELMET
  // helmet
  auto helmet = _ftScene->addSingleTexturedFromGltf(
      "assets/models/FlightHelmet/glTF/"
      "FlightHelmet.gltf",
      _ftTexturedRdrSys->getDescriptorPool(),
      _ftTexturedRdrSys->getDescriptorSetLayout());
  for (const auto &model : helmet) {
    glm::mat4 &mat = model->getRootModelMatrix();
    mat = glm::rotate(mat, glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    mat = glm::translate(mat, glm::vec3(5.0f, 0.45f, 10.0f));
    model->setFlags(model->getID(), ft::MODEL_SELECTABLE_BIT);
  }
#endif
#if SHOW_SPONZA
  // sponza
  auto sponza = _ftScene->addDoubleTexturedFromGltf(
      "assets/models/sponza/sponza.gltf",
      _ft2TexturedRdrSys->getDescriptorPool(),
      _ft2TexturedRdrSys->getDescriptorSetLayout());
  std::cout << "loaded sponza " << sponza.size() << std::endl;
  for (const auto &model : sponza) {
    glm::mat4 &mat = model->getRootModelMatrix();
    mat = glm::rotate(mat, glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    mat = glm::translate(mat, glm::vec3(0.0f, 0.45f, 0.0f));
    mat = glm::scale(mat, {12.0f, 12.0f, 12.0f});
    model->setFlags(model->getID(), ft::MODEL_SELECTABLE_BIT);
  }
#endif
#if SHOW_VIKINGS
  // viking's room
  data = {};
  data.color = {0.0f, 0.9f, 0.2f};
  // data.rotation = glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), {1, 0,
  // 0}); data.translation = glm::translate(glm::mat4(1.0f), {10.0f, 0.0f,
  // 0.0f});
  auto room = _ftScene->addModelFromObj("assets/models/viking_room.obj", data);
  room->setFlags(room->getID(),
                 ft::MODEL_SIMPLE_BIT | ft::MODEL_SELECTABLE_BIT);

  auto t = _ftMaterialPool->createTexture(
      "assets/textures/viking_room.png", ft::Texture::FileType::FT_TEXTURE_PNG);
  auto material = std::make_shared<Material>(_ftDevice);
  material->addTexture(t);
  material->createDescriptors(_ftTexturedRdrSys->getDescriptorPool(),
                              _ftTexturedRdrSys->getDescriptorSetLayout());
  for (int i = 0; i < ft::MAX_FRAMES_IN_FLIGHT; ++i) {
    material->bindDescriptor(i, 0, 1);
  }
  _ftMaterialPool->addMaterial(material);
  room->addMaterial(material);
  room->unsetFlags(room->getID(), ft::MODEL_SIMPLE_BIT);
  room->setFlags(room->getID(),
                 ft::MODEL_HAS_COLOR_TEXTURE_BIT | ft::MODEL_SELECTABLE_BIT);
#endif
#if SHOW_VENUS
  // venus
  data = {};
  data.color = {0.95f, .9f, .5f};
  data.rotation = glm::rotate(glm::mat4(1.0f), glm::radians(180.0f),
                              glm::vec3(0.0f, 1.0f, 1.0f));
  data.translation = glm::translate(glm::mat4(1.0f), {0.0, 0.0, -5.7f});
  data.translation = glm::translate(glm::mat4(1.0f), {10, 10, 10});
  auto venus = _ftScene->addModelFromGltf("assets/models/venus.gltf", data);
  venus[0]->setFlags(venus[0]->getID(), ft::MODEL_SELECTABLE_BIT);
#endif

#if SHOW_CAR
  data.color = {0.5f, 0.6f, 0.9f};
  data.translation = glm::translate(glm::mat4(1.0f), {0, 0, 3});
  data.rotation = glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), {1, 0, 0});
  data.scaling = glm::scale(glm::mat4(1.0f), {0.005, 0.005, 0.005});
  auto car = _ftScene->addModelFromObj("assets/models/car.obj", data);
  car->setFlags(car->getID(), ft::MODEL_SIMPLE_BIT | ft::MODEL_SELECTABLE_BIT);

#endif

#if SHOW_ARROW
  data.color = {0.9f, 0.1f, 0.2f};
  // data.model = glm::translate(data.model, {0, 0,
  // 3}); data.model = glm::rotate(data.model,
  // glm::radians(180.0f), {1, 0, 0});
  auto arrow = _ftScene->addModelFromObj("assets/models/arrow.obj", data);
  arrow->setFlags(arrow->getID(),
                  ft::MODEL_SIMPLE_BIT | ft::MODEL_SELECTABLE_BIT);

#endif
#if SHOW_UNIT_BOX
  data = {};
  data.color = {0.5f, 0.3f, 0.2f};
  //  auto unit_box = _ftScene->addModelFromObj("models/unit_box.obj", data);
  auto unit_box =
      _ftScene->addModelFromGltf("assets/models/unit_box.gltf", data);
  unit_box[0]->setFlags(unit_box[0]->getID(),
                        ft::MODEL_SIMPLE_BIT | ft::MODEL_SELECTABLE_BIT);

#endif
#if SHOW_GIZMO
  data = {};
  data.color = {0.9f, 0.1f, 0.2f};
  auto h = _ftScene->addModelFromGltf("assets/models/gizmo.gltf", data);
  h[0]->setFlags(h[0]->getID(),
                 ft::MODEL_SIMPLE_BIT | ft::MODEL_SELECTABLE_BIT);
#endif
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

  _ftScene->drawSkyBox(commandBuffer, _ftSkyBoxRdrSys->getGraphicsPipeline(),
                       _ftSkyBoxRdrSys, _currentFrame);
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

  _ftScene->drawNormals(commandBuffer, _ftSimpleRdrSys, _ftNormDebugRdrSys,
                        _currentFrame);

  // _ftScene->drawSimpleObjsWithOutline(commandBuffer, _ftSimpleRdrSys,
  //                                     _ftOutlineRdrSys, _currentFrame);

  // _ftScene->drawOulines(commandBuffer, _ftSimpleRdrSys, _ftOutlineRdrSys,
  //                       _currentFrame);

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
    _ftScene->unselectAll();
    std::cout << "unselect all\n";
  } else if (key == _ftWindow->KEY(KeyboardKeys::KEY_H)) {
    _ftScene->hideSelected();
  } else if (key == _ftWindow->KEY(KeyboardKeys::KEY_Y)) {
    _ftScene->unhideAll();
  } else if (key == _ftWindow->KEY(KeyboardKeys::KEY_T)) {
    _ftScene->resetAll();
    std::cout << "reset all\n";
  } else if (key == _ftWindow->KEY(KeyboardKeys::KEY_P)) {
    _ftScene->togglePointsTopo();
  } else if (key == _ftWindow->KEY(KeyboardKeys::KEY_L)) {
    _ftScene->toggleLinesTopo();
  } else if (key == _ftWindow->KEY(KeyboardKeys::KEY_F4)) {
    std::cout << "calculating the normals " << std::endl;
    _ftScene->calculateNormals();
  } else if (key == _ftWindow->KEY(KeyboardKeys::KEY_N)) {
    _ftScene->toggleNormalDebug();
  } else if (key == _ftWindow->KEY(KeyboardKeys::KEY_B)) {
    _ftScene->toggleGizmo();
  } else if (key == _ftWindow->KEY(KeyboardKeys::KEY_I)) {
    std::cout << "info: \n";
    std::cout << "sizeof pushconst: " << sizeof(PushConstantObject)
              << std::endl;
    _ftScene->showSelectedInfo();
  } else if (key == _ftWindow->KEY(KeyboardKeys::KEY_U)) {
    _ftPhysicsApplication->play();
    _play = true;
  } else if (key == _ftWindow->KEY(KeyboardKeys::KEY_O)) {
    _ftPhysicsApplication->pause();
    _play = false;
  }

  _ftScene->updateCameraUBO();
  _ftMousePicker->notifyUpdatedView();
}

void ft::Application::setScenePath(const std::string &path) {
  _scenePath = path;

  if (!_scenePath.empty()) {
    _ftJsonParser->parseSceneFile(_ftScene, _scenePath,
                                  _ftRenderer->getSwapChain()->getAspect());
  } else {
    std::cout << "scene file empty!" << std::endl;
  }
}

// todo! reimplement for multi-threading
void ft::Application::checkEventQueue() {
  while (!_ftEventListener->isQueueEmpty()) {
    auto ev = _ftEventListener->popEvent();
    _ftThreadPool->addTask([&, e = ev.release()]() mutable {
      _ftEventListener->fireInstante(*e);
      delete e;
    });
  }
}
