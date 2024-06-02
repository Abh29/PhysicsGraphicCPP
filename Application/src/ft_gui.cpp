#include "../includes/ft_gui.h"
#include "../imgui/imgui_internal.h"
#include "../includes/ft_rigidComponent.h"
#include "../includes/ft_rigidObject.h"
#include "ft_defines.h"
#include "ft_scene.h"
#include <cstdint>
#include <glm/fwd.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iomanip>
#include <memory>
#include <sstream>
#include <string>

ft::Gui::Gui(Instance::pointer instance, PhysicalDevice::pointer physicalDevice,
             Device::pointer device, Window::pointer window,
             RenderPass::pointer renderPass, uint32_t imageCount,
             ft::GlobalState &global)
    : _ftInstance(instance), _ftPhysicalDevice(physicalDevice),
      _ftDevice(device), _ftWindow(window), _ftRenderPass(renderPass),
      _ftGlobalState(global) {

  // 1: create descriptor pool for IMGUI
  VkDescriptorPoolSize pool_sizes[] = {
      {VK_DESCRIPTOR_TYPE_SAMPLER, 1000},
      {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000},
      {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000},
      {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000},
      {VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000},
      {VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000},
      {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000},
      {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000},
      {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000},
      {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000},
      {VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000}};

  VkDescriptorPoolCreateInfo pool_info = {};
  pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
  pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
  pool_info.maxSets = 1000;
  pool_info.poolSizeCount = std::size(pool_sizes);
  pool_info.pPoolSizes = pool_sizes;

  if (vkCreateDescriptorPool(_ftDevice->getVKDevice(), &pool_info, nullptr,
                             &_descriptorPool) != VK_SUCCESS) {
    throw std::runtime_error("can not create a descriptor pool for imgui!");
  }

  // 2: initialize imgui library for glfw
  ImGui::CreateContext();
  ImGui::StyleColorsDark();
  ImGui_ImplGlfw_InitForVulkan((GLFWwindow *)_ftWindow->getRawWindowPointer(),
                               true);

  // init imgui for vulkan
  ImGui_ImplVulkan_InitInfo init_info = {};
  init_info.Instance = _ftInstance->getVKInstance();
  init_info.PhysicalDevice = _ftPhysicalDevice->getVKPhysicalDevice();
  init_info.Device = _ftDevice->getVKDevice();
  init_info.QueueFamily =
      _ftDevice->getQueueFamilyIndices().graphicsFamily.value();
  init_info.Queue = _ftDevice->getVKGraphicsQueue();
  init_info.DescriptorPool = _descriptorPool;
  init_info.Subpass = 0;
  init_info.MinImageCount = imageCount;
  init_info.ImageCount = imageCount;
  init_info.MSAASamples = _ftDevice->getMSAASamples();
  ImGui_ImplVulkan_Init(&init_info, _ftRenderPass->getVKRenderPass());

  // load fonts
  auto &io = ImGui::GetIO();
  io.Fonts->AddFontFromFileTTF("misc/fonts/Roboto-Medium.ttf", 14.0f, nullptr,
                               io.Fonts->GetGlyphRangesCyrillic());

  CommandBuffer::pointer commandBuffer =
      std::make_shared<CommandBuffer>(_ftDevice);
  commandBuffer->beginRecording(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
  ImGui_ImplVulkan_CreateFontsTexture(commandBuffer->getVKCommandBuffer());
  commandBuffer->end();
  commandBuffer->submit(_ftDevice->getVKGraphicsQueue());

  // clear font textures from host memory
  ImGui_ImplVulkan_DestroyFontUploadObjects();
}

ft::Gui::~Gui() {
  vkDestroyDescriptorPool(_ftDevice->getVKDevice(), _descriptorPool, nullptr);
  ImGui_ImplVulkan_Shutdown();
}

// Helper to wire demo markers located in code to an interactive browser
typedef void (*ImGuiDemoMarkerCallback)(const char *file, int line,
                                        const char *section, void *user_data);
extern ImGuiDemoMarkerCallback GImGuiDemoMarkerCallback;
extern void *GImGuiDemoMarkerCallbackUserData;
#define IMGUI_DEMO_MARKER(section)                                             \
  do {                                                                         \
    if (GImGuiDemoMarkerCallback != NULL)                                      \
      GImGuiDemoMarkerCallback(__FILE__, __LINE__, section,                    \
                               GImGuiDemoMarkerCallbackUserData);              \
  } while (0)

// Helper to display a little (?) mark which shows a tooltip when hovered.
// In your own code you may want to display an actual icon if you are using a
// merged icon fonts (see docs/FONTS.md)
[[maybe_unused]] static void HelpMarker(const char *desc) {
  ImGui::TextDisabled("(?)");
  if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayShort) &&
      ImGui::BeginTooltip()) {
    ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
    ImGui::TextUnformatted(desc);
    ImGui::PopTextWrapPos();
    ImGui::EndTooltip();
  }
}
void ft::Gui::newFrame() {
  ImGui_ImplVulkan_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();
}

void ft::Gui::showDemo() { ImGui::ShowDemoWindow(); }

void ft::Gui::showGUI(const ft::Scene::pointer &scene, uint32_t flags,
                      bool *p_open) {
  (void)flags;
  (void)p_open;
  showTitleBar();
  showMainMenue(scene);
  if (show_app_metrics)

    showMetrics(&show_app_metrics);
  if (show_app_about)
    showAboutWindow(&show_app_about);
}

void ft::Gui::render(ft::CommandBuffer::pointer commandBuffer) {
  ImGui::Render();
  ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(),
                                  commandBuffer->getVKCommandBuffer());
}

void ft::Gui::showMetrics(bool *p_open) {
  auto context = ImGui::GetCurrentContext();

  ImGuiIO &io = context->IO;
  ImGuiMetricsConfig *cfg = &context->DebugMetricsConfig;
  (void)cfg;

  ImGuiWindowFlags window_flags = 0;
  window_flags |= ImGuiWindowFlags_NoScrollbar;
  //    window_flags |= ImGuiWindowFlags_NoMove;
  window_flags |= ImGuiWindowFlags_NoResize;
  window_flags |= ImGuiWindowFlags_NoCollapse;
  window_flags |= ImGuiWindowFlags_NoNav;

  const ImGuiViewport *main_viewport = ImGui::GetMainViewport();
  ImGui::SetNextWindowPos(ImVec2(0, main_viewport->Size.y - 65),
                          ImGuiCond_Always);

  // Main body of the Demo window starts here.
  if (!ImGui::Begin("Metrics", p_open, window_flags)) {
    ImGui::End();
    return;
  }

  // Basic info
  ImGui::Text("Dear ImGui %s", ImGui::GetVersion());
  ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
              1000.0f / io.Framerate, io.Framerate);

  ImGui::End();
}

void ft::Gui::showTitleBar() {
  IM_ASSERT(ImGui::GetCurrentContext() != NULL &&
            "Missing dear imgui context. Refer to examples app!");

  static bool no_titlebar = true;
  static bool no_scrollbar = false;
  static bool no_menu = false;
  static bool no_move = false;
  static bool no_resize = true;
  static bool no_collapse = false;
  static bool no_nav = false;
  static bool no_background = false;
  static bool no_bring_to_front = false;
  static bool unsaved_document = false;

  ImGuiWindowFlags window_flags = 0;
  if (no_titlebar)
    window_flags |= ImGuiWindowFlags_NoTitleBar;
  if (no_scrollbar)
    window_flags |= ImGuiWindowFlags_NoScrollbar;
  if (!no_menu)
    window_flags |= ImGuiWindowFlags_MenuBar;
  if (no_move)
    window_flags |= ImGuiWindowFlags_NoMove;
  if (no_resize)
    window_flags |= ImGuiWindowFlags_NoResize;
  if (no_collapse)
    window_flags |= ImGuiWindowFlags_NoCollapse;
  if (no_nav)
    window_flags |= ImGuiWindowFlags_NoNav;
  if (no_background)
    window_flags |= ImGuiWindowFlags_NoBackground;
  if (no_bring_to_front)
    window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus;
  if (unsaved_document)
    window_flags |= ImGuiWindowFlags_UnsavedDocument;

  // We specify a default position/size in case there's no data in the .ini
  // file. We only do it to make the demo applications a little more welcoming,
  // but typically this isn't required.
  const ImGuiViewport *main_viewport = ImGui::GetMainViewport();
  ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
  ImGui::SetNextWindowSize(ImVec2(main_viewport->Size.x, 30), ImGuiCond_Always);

  // Main body of the Demo window starts here.
  if (!ImGui::Begin("FTGraphics", nullptr, window_flags)) {
    // Early out if the window is collapsed, as an optimization.
    ImGui::End();
    return;
  }

  // Most "big" widgets share a common width settings by default. See
  // 'Demo->Layout->Widgets Width' for details. e.g. Use 2/3 of the space for
  // widgets and 1/3 for labels (right align)
  // ImGui::PushItemWidth(-ImGui::GetWindowWidth() * 0.35f);
  // e.g. Leave a fixed amount of width for labels (by passing a negative
  // value), the rest goes to widgets.
  ImGui::PushItemWidth(ImGui::GetFontSize() * -12);

  // Menu Bar
  if (ImGui::BeginMenuBar()) {
    if (ImGui::BeginMenu("File")) {
      IMGUI_DEMO_MARKER("Menu/File");
      showExampleMenuFile();
      ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("Insert")) {
      IMGUI_DEMO_MARKER("Menu/Insert");
      showInsertMenueFile();
      ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("Edit")) {
      IMGUI_DEMO_MARKER("Menu/Edit");
      showEditMenueFile();
      ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("Tools")) {
      IMGUI_DEMO_MARKER("Menu/Tools");
      showToolsMenueFile();
      ImGui::EndMenu();
    }
    ImGui::EndMenuBar();
  }

  // End of ShowDemoWindow()
  ImGui::PopItemWidth();
  ImGui::End();
}

static void displayMat4(const glm::mat4 &m) {

  ImGui::BeginDisabled(true);                // Begin disabled section
  ImGui::Columns(4, "matrixColumns", false); // 4 columns for 4x4 matrix

  for (int col = 0; col < 4; ++col) {
    for (int row = 0; row < 4; ++row) {
      // Display the matrix element
      ImGui::Text("%f", m[col][row]);
    }
    ImGui::NextColumn();
  }

  ImGui::Columns(1);    // Reset to default
  ImGui::EndDisabled(); // End disabled section
}

void ft::Gui::showMainMenue(const ft::Scene::pointer &scene) {
  (void)scene;
  IM_ASSERT(ImGui::GetCurrentContext() != NULL &&
            "Missing dear imgui context. Refer to examples app!");

  ImGuiWindowFlags window_flags = 0;
  window_flags |= ImGuiWindowFlags_NoTitleBar;
  window_flags |= ImGuiWindowFlags_NoScrollbar;
  window_flags |= ImGuiWindowFlags_NoMove;
  window_flags |= ImGuiWindowFlags_NoResize;
  window_flags |= ImGuiWindowFlags_NoCollapse;
  window_flags |= ImGuiWindowFlags_NoNav;

  // We specify a default position/size in case there's no data in the .ini
  // file. We only do it to make the demo applications a little more welcoming,
  // but typically this isn't required.
  const ImGuiViewport *main_viewport = ImGui::GetMainViewport();
  ImGui::SetNextWindowPos(ImVec2(main_viewport->Size.x - 200, 31),
                          ImGuiCond_Always);
  ImGui::SetNextWindowSize(ImVec2(200, main_viewport->Size.y - 30),
                           ImGuiCond_Always);

  // Main body of the Demo window starts here.
  if (!ImGui::Begin("main_menue", nullptr, window_flags)) {
    // Early out if the window is collapsed, as an optimization.
    ImGui::End();
    return;
  }

  ImGui::PushItemWidth(ImGui::GetFontSize() * -12);

  // Scene Controls
  if (ImGui::CollapsingHeader("Scene Cameras")) {
    ImGui::Text("Camera Settings:");
    auto &cams = scene->getAllCameras();
    float aspect = scene->getCamera()->getAspect();
    uint32_t index = 0;

    for (auto &cam : cams) {
      auto &pos = cam->getEyePosition();
      ImGui::SetNextItemOpen(scene->getCamera() == cam);
      float fov = cam->getFov();
      // float aspect = cam->getAspect();

      if (ImGui::TreeNode(
              std::string("camera " + std::to_string(index++) + ":").c_str())) {

        ImGui::Text("Eye Position: ");
        ImGui::BeginDisabled(true); // Begin disabled section
        ImGui::PushItemWidth(-FLT_MIN);
        ImGui::DragFloat3("##eye", glm::value_ptr(pos));
        ImGui::PopItemWidth();
        ImGui::EndDisabled(); // End disabled section

        ImGui::Text("Field Of View: ");
        ImGui::PushItemWidth(-FLT_MIN);
        ImGui::SliderFloat("##fov", &fov, 30.0f, 120.0f, "%.1f");
        ImGui::PopItemWidth();

        ImGui::TreePop();

        if (fov != cam->getFov()) {
          cam->setFov(fov);
          scene->updateCameraUBO();
        }
      }
    }

    ImGui::Text("Asspect: ");
    ImGui::BeginDisabled(true); // Begin disabled section
    ImGui::PushItemWidth(-FLT_MIN);
    ImGui::DragFloat("##aspect", &aspect);
    ImGui::EndDisabled(); // End disabled section

    ImGui::Text("Point size: ");
    if (ImGui::SliderFloat("##pointsize", &(scene->getUBO().pointSize), 0.0f,
                           4.0f, "%.1f"))
      scene->updateCameraUBO();

    ImGui::Separator();
    if (ImGui::TreeNode("Projection Matrix: ")) {

      auto &m = scene->getUBO().proj;
      displayMat4(m);
      ImGui::TreePop();
    }

    if (ImGui::TreeNode("View Matrix: ")) {

      auto &m = scene->getUBO().view;
      displayMat4(m);
      ImGui::TreePop();
    }

    ImGui::Separator();
  }

  if (ImGui::CollapsingHeader("Scene Light")) {

    ImGui::Text("Lighting Settings:");
    ImGui::PushItemWidth(-FLT_MIN);
    bool updated = false;
    auto &ubo = scene->getUBO();

    updated = ImGui::ColorEdit3("##lightColor", glm::value_ptr(ubo.lightColor));

    updated |=
        ImGui::SliderFloat("##ambient", &(ubo.ambient), 0.0f, 1.0f, "%.2f");

    ImGui::Text("Light position: ");
    updated |=
        ImGui::DragFloat3("##position", glm::value_ptr(ubo.lightDirection));

    auto lights = scene->getLights();
    for (uint32_t i = 0; i < ubo.pLCount; ++i) {

      if (ImGui::TreeNode(
              std::string("PointLight" + std::to_string(i) + ":").c_str())) {

        updated |= ImGui::Checkbox("Active: ",
                                   reinterpret_cast<bool *>(&(lights[i].on)));
        ImGui::Text("Position: ");
        ImGui::PushItemWidth(-FLT_MIN);
        updated |= ImGui::DragFloat3("##pLPosition",
                                     glm::value_ptr(lights[i].position));
        ImGui::PopItemWidth();

        ImGui::Text("Color: ");
        ImGui::PushItemWidth(-FLT_MIN);
        updated |=
            ImGui::ColorEdit3("##pLColor", glm::value_ptr(lights[i].color));
        ImGui::PopItemWidth();

        ImGui::Text("constant:");
        updated |= ImGui::SliderFloat("##pLconst", &(lights[i].attenuation.x),
                                      0.5f, 1.5f, "%.2f");

        ImGui::Text("linear:");
        updated |= ImGui::SliderFloat("##pLlinear", &(lights[i].attenuation.y),
                                      0.01f, 0.30f, "%.2f");

        ImGui::Text("quadratic:");
        updated |= ImGui::SliderFloat("##pLquad", &(lights[i].attenuation.z),
                                      0.01f, 0.1f, "%.3f");

        ImGui::Text("ambient:");
        updated |= ImGui::SliderFloat("##pLambient", &(lights[i].ambient), 0.0f,
                                      1.0f, "%.2f");

        ImGui::Text("diffuse:");
        updated |= ImGui::SliderFloat("##pLdiff", &(lights[i].diffuse), 0.0f,
                                      1.0f, "%.2f");

        ImGui::Text("specular:");
        updated |= ImGui::SliderFloat("##pLspec", &(lights[i].specular), 0.0f,
                                      1.0f, "%.2f");
        ImGui::TreePop();
      }
    }

    if (updated) {
      scene->updateCameraUBO();
    }

    // Add more lighting controls as needed
  }

  // objects

  if (ImGui::CollapsingHeader("Scene Objects: ")) {

    auto &graph = scene->getSceneGraph();

    for (auto &gr : graph) {

      ImGui::Text(
          "%s",
          (gr._inputFile.substr(gr._inputFile.find_last_of('/') + 1)).c_str());

      for (auto &model : gr._models) {
        if (model->hasFlag(ft::MODEL_SELECTED_BIT))
          ImGui::SetNextItemOpen(true);
        if (ImGui::TreeNode(
                (std::string("Model ") + std::to_string(model->getID()))
                    .c_str())) {

          bool selected = model->hasFlag(ft::MODEL_SELECTED_BIT);
          if (model->hasFlag(ft::MODEL_SELECTABLE_BIT) &&
              ImGui::Checkbox("Selected", &selected)) {
            scene->select(model->getID());
          }

          bool hidden = model->hasFlag(ft::MODEL_HIDDEN_BIT);
          if (ImGui::Checkbox("Hide", &hidden)) {
            if (hidden)
              model->setFlags(model->getID(), ft::MODEL_HIDDEN_BIT);
            else
              model->unsetFlags(model->getID(), ft::MODEL_HIDDEN_BIT);
          }

          bool nDbg = model->hasFlag(ft::MODEL_HAS_NORMAL_DEBUG_BIT);
          if (ImGui::Checkbox("Show Normals", &nDbg)) {
            if (nDbg)
              model->setFlags(model->getID(), ft::MODEL_HAS_NORMAL_DEBUG_BIT);
            else
              model->unsetFlags(model->getID(), ft::MODEL_HAS_NORMAL_DEBUG_BIT);
          }

          ImGui::BeginDisabled();
          bool simple = model->hasFlag(ft::MODEL_SIMPLE_BIT);
          ImGui::Checkbox("Simple", &simple);

          bool cTex = model->hasFlag(ft::MODEL_HAS_COLOR_TEXTURE_BIT);
          if (ImGui::Checkbox("Color Texture", &cTex)) {
            if (!cTex) {
              model->unsetFlags(model->getID(),
                                ft::MODEL_HAS_COLOR_TEXTURE_BIT);
              model->setFlags(model->getID(), ft::MODEL_SIMPLE_BIT);
            } else {
              model->unsetFlags(model->getID(), ft::MODEL_SIMPLE_BIT);
              model->setFlags(model->getID(), ft::MODEL_HAS_COLOR_TEXTURE_BIT);
            }
          }

          bool nTex = model->hasFlag(ft::MODEL_HAS_NORMAL_TEXTURE_BIT);
          if (ImGui::Checkbox("Normal Texture", &nTex)) {
            if (!nTex) {
              model->unsetFlags(model->getID(),
                                ft::MODEL_HAS_NORMAL_TEXTURE_BIT);
              model->setFlags(model->getID(), ft::MODEL_SIMPLE_BIT);
            } else {
              model->unsetFlags(model->getID(), ft::MODEL_SIMPLE_BIT);
              model->setFlags(model->getID(), ft::MODEL_HAS_NORMAL_TEXTURE_BIT);
            }
          }
          ImGui::EndDisabled();

          if (ImGui::TreeNode("Transform")) {
            displayMat4(model->getRootModelMatrix());
            ImGui::TreePop();
          }

          if (model->getAllNodes().size() > 1) {

            for (auto &n : model->getAllNodes()) {
              if (ImGui::TreeNode((std::string("Node ") +
                                   std::to_string(n->state.id) +
                                   std::string(": "))
                                      .c_str())) {

                ImGui::PushItemWidth(-FLT_MIN);
                ImGui::ColorEdit3("##NodeColor",
                                  glm::value_ptr(n->state.baseColor));
                bool hide = n->state.flags & ft::MODEL_HIDDEN_BIT;
                if (ImGui::Checkbox("Hide: ", &hide)) {
                  if (hide)
                    n->state.flags |= ft::MODEL_HIDDEN_BIT;
                  else
                    n->state.flags &= ~ft::MODEL_HIDDEN_BIT;
                }

                ImGui::PopItemWidth();

                for (auto &p : n->mesh) {
                  if (ImGui::TreeNode("Primitive")) {
                    (void)p;
                    ImGui::Text("indices: %d\nmaterial index: %d", p.indexCount,
                                p.materialIndex);
                    ImGui::TreePop();
                  }
                }
                ImGui::TreePop();
              }
            }
          } else {

            ImGui::PushItemWidth(-FLT_MIN);
            ImGui::ColorEdit3(
                "##BasicColor",
                glm::value_ptr(model->getAllNodes()[0]->state.baseColor));
            ImGui::PopItemWidth();
          }

          if (model->hasFlag(ft::MODEL_HAS_RIGID_BODY_BIT)) {
            if (ImGui::TreeNode("Physics: ")) {

              auto objs = scene->getObjects();
              SceneObject::pointer obj = nullptr;
              // todo change this to take the scene model from the graph
              for (auto &i : objs) {
                if (i->getModel() == model) {
                  obj = i;
                  break;
                }
              }

              if (obj.get()) {
                RigidBallComponent::raw_ptr rball = nullptr;
                RigidBoxComponent::raw_ptr rbox = nullptr;

                rball = obj->getComponent<RigidBallComponent>();
                if (rball) {
                  ImGui::Text("Sphere Collider");
                  bool pause = rball->getPause();
                  if (ImGui::Checkbox("pause", &pause)) {
                    rball->setPause(pause);
                  }

                  // rball->backwardUpdate(1.0f);
                }

                rbox = obj->getComponent<RigidBoxComponent>();
                if (rbox) {
                  ImGui::Text("Cube Collider");
                  bool pause = rbox->getPause();
                  if (ImGui::Checkbox("pause", &pause)) {
                    rbox->setPause(pause);
                  }

                  // rbox->backwardUpdate(1.0f);
                }
              }

              ImGui::TreePop();
            }
          } else {
            // todo: add physics component
          }

          ImGui::TreePop();
        }
      }

      ImGui::Separator();
    }
    // Add more lighting controls as needed
  }

  // Simulation Controls
  if (ImGui::CollapsingHeader("Simulation Controls")) {
    if (ImGui::Button("Play")) { /* Play simulation */
    }
    ImGui::SameLine();
    if (ImGui::Button("Pause")) { /* Pause simulation */
    }
    ImGui::SameLine();
    if (ImGui::Button("Reset")) { /* Reset simulation */
    }
    // Add more simulation controls as needed
  }

  // Performance Metrics
  if (ImGui::CollapsingHeader("Performance Metrics")) {
    ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
    // Add more performance metrics as needed
  }

  // Debugging Tools
  if (ImGui::CollapsingHeader("Debugging Tools")) {
    static bool showBoundingBoxes = false;
    ImGui::Checkbox("Show Bounding Boxes", &showBoundingBoxes);
    // Add more debugging tools as needed
  }

  // Settings and Preferences
  if (ImGui::CollapsingHeader("Settings and Preferences")) {
    static bool darkTheme = true;
    if (ImGui::Checkbox("Dark Theme", &darkTheme)) {
      if (darkTheme) {
        ImGui::StyleColorsDark();
      } else {
        ImGui::StyleColorsLight();
      }
    }
    // Add more settings and preferences as needed
  }

  // End of ShowDemoWindow()
  ImGui::PopItemWidth();
  ImGui::End();
}

void ft::Gui::showExampleMenuFile() {

  if (ImGui::MenuItem("New", nullptr, false, false)) {

    _ftWindow->getEventListener()->pushEvent(
        std::make_unique<StandardEvent>(Event::EventType::Menue_File_NEW));
  }

  if (ImGui::MenuItem("Open", nullptr, false, false)) {
    _ftWindow->getEventListener()->pushEvent(
        std::make_unique<StandardEvent>(Event::EventType::Menue_File_OPEN));
  }

  if (ImGui::MenuItem("Save", nullptr, false, true)) {
    _ftWindow->getEventListener()->pushEvent(
        std::make_unique<StandardEvent>(Event::EventType::Menue_File_SAVE));
  }

  if (ImGui::MenuItem("Save As..", nullptr, false, false)) {
    _ftWindow->getEventListener()->pushEvent(
        std::make_unique<StandardEvent>(Event::EventType::Menue_File_SAVEAS));
  }

  if (ImGui::MenuItem("Reload from save", nullptr, false, true)) {
    auto e = StandardEvent(Event::EventType::Menue_File_RELOAD);
    _ftWindow->getEventListener()->fireInstante(e);
  }

  ImGui::Separator();

  if (ImGui::BeginMenu("Disabled", false)) // Disabled
  {
    IM_ASSERT(0);
  }
  ImGui::Separator();
  if (ImGui::MenuItem("Quit", "Alt+F4")) {
    _ftWindow->close();
  }
}

void ft::Gui::showInsertMenueFile() {
  ImGui::Separator();
  if (ImGui::MenuItem("New Model", nullptr, false, false)) {
    _ftWindow->getEventListener()->pushEvent(
        std::make_unique<StandardEvent>(Event::EventType::Menue_Insert_MODEL));
  }

  // if (ImGui::MenuItem("New Texture")) {}
  ImGui::Separator();
  if (ImGui::MenuItem("SkyBox", nullptr, false, false)) {
    _ftWindow->getEventListener()->pushEvent(
        std::make_unique<StandardEvent>(Event::EventType::Menue_Insert_SKYBOX));
  }

  ImGui::Separator();

  if (ImGui::MenuItem("Collision Plane", nullptr, false)) {

    auto e = StandardEvent(Event::EventType::Menue_Insert_USPHERE);
    _ftWindow->getEventListener()->fireInstante(e);
    // _ftWindow->getEventListener()->pushEvent(
    //     std::make_unique<StandardEvent>(Event::EventType::Menue_Insert_UBOX));
  }

  if (ImGui::MenuItem("Unit Cube")) {

    auto e = StandardEvent(Event::EventType::Menue_Insert_UBOX);
    _ftWindow->getEventListener()->fireInstante(e);
    // _ftWindow->getEventListener()->pushEvent(
    //     std::make_unique<StandardEvent>(Event::EventType::Menue_Insert_UBOX));
  }

  if (ImGui::MenuItem("Unit Sphere")) {

    auto e = StandardEvent(Event::EventType::Menue_Insert_USPHERE);
    _ftWindow->getEventListener()->fireInstante(e);

    //   _ftWindow->getEventListener()->pushEvent(std::make_unique<StandardEvent>(
    //     Event::EventType::Menue_Insert_USPHERE));
  }

  ImGui::Separator();

  if (ImGui::MenuItem("Camera")) {
    auto e = StandardEvent(Event::EventType::Menue_Insert_Camera);
    _ftWindow->getEventListener()->fireInstante(e);
  }

  if (ImGui::MenuItem("Point Light")) {
    auto e = StandardEvent(Event::EventType::Menue_Insert_PLight);
    _ftWindow->getEventListener()->fireInstante(e);
  }
}

void ft::Gui::showToolsMenueFile() {
  ImGui::MenuItem("Metrics", NULL, &show_app_metrics, true);
  ImGui::MenuItem("About", NULL, &show_app_about);
}

void ft::Gui::showEditMenueFile() {
  if (ImGui::MenuItem("Unselect All")) {
    _ftWindow->getEventListener()->pushEvent(std::make_unique<StandardEvent>(
        Event::EventType::Menue_Edit_UNSELECTALL));
  }
  if (ImGui::MenuItem("Unhide All")) {
    _ftWindow->getEventListener()->pushEvent(std::make_unique<StandardEvent>(
        Event::EventType::Menue_Edit_UNHIDEALL));
  }

  ImGui::Separator();

  if (ImGui::MenuItem("Default Topology")) {
    _ftWindow->getEventListener()->pushEvent(std::make_unique<StandardEvent>(
        Event::EventType::Menue_Edit_DEFAULTTOPO));
  }
  if (ImGui::MenuItem("Line Topology")) {
    _ftWindow->getEventListener()->pushEvent(
        std::make_unique<StandardEvent>(Event::EventType::Menue_Edit_LINETOPO));
  }
  if (ImGui::MenuItem("Point Topology")) {
    _ftWindow->getEventListener()->pushEvent(
        std::make_unique<StandardEvent>(Event::EventType::Menue_Edit_POINTTOP));
  }

  ImGui::Separator();

  if (ImGui::MenuItem("Recalculate Normals")) {
    _ftWindow->getEventListener()->pushEvent(std::make_unique<StandardEvent>(
        Event::EventType::Menue_Edit_RECALCNORM));
  }
  if (ImGui::MenuItem("Show Normals")) {
    _ftWindow->getEventListener()->pushEvent(
        std::make_unique<StandardEvent>(Event::EventType::Menue_Edit_SHOWNORM));
  }

  ImGui::Separator();

  if (ImGui::MenuItem("Next Camera")) {
    _ftWindow->getEventListener()->pushEvent(std::make_unique<StandardEvent>(
        Event::EventType::Menue_Edit_NEXTCAMERA));
  }
  if (ImGui::MenuItem("Remove Camera")) {
    _ftWindow->getEventListener()->pushEvent(std::make_unique<StandardEvent>(
        Event::EventType::Menue_Edit_REMOVECAMERA));
  }

  // if (ImGui::MenuItem("bake transforms")) {
  // }
}

bool ft::Gui::isGuiHovered() const { return ImGui::IsAnyItemHovered(); }

bool ft::Gui::isMouseCaptured() const {
  return ImGui::GetIO().WantCaptureMouse;
}

bool ft::Gui::isKeyCaptured() const {
  return ImGui::GetIO().WantCaptureKeyboard;
}

float ft::Gui::getFramerate() const {
  return ImGui::GetCurrentContext()->IO.Framerate;
}

void ft::Gui::showAboutWindow(bool *p_open) {
  if (!ImGui::Begin("About", p_open, ImGuiWindowFlags_AlwaysAutoResize)) {
    ImGui::End();
    return;
  }
  IMGUI_DEMO_MARKER("Tools/About");
  ImGui::Text("FtGraphics version: 1.0.0");
  ImGui::Separator();

  ImGui::Text(
      u8"\tЭто демонстрационное приложение, демонстрирующее использование графического \n\
серверного средства визуализации, основанного на Vulkan, и физического \n\
серверного движка для динамического анализа твердых тел, основанного на \n\
книге под названием 'Разработка игрового физического движка - Ян Миллингтон',\n\
оно предназначено в качестве отправной точки с точки зрения специалиста по \n\
компьютерной графике, демонстрирующего примеры алгоритмов и методик.\n\n");

  ImGui::Text(
      u8"\tРендерер поддерживает как obj, так и gif-файлы, а также текстуры в формате \n\
png и ktx с различными конвейерами, а также пользовательскую систему для \n\
многопоточной обработки событий.\n\n");

  ImGui::Text(
      u8"\tФизический движок поддерживает динамику твердого тела и физику \n\
элементарных частиц (хотя и не показан в этой демонстрации) с акцентом \n\
на простой и улучшаемый дизайн.\n\n");

  ImGui::Text(
      u8"\tЭта демонстрация сделана с использованием imgui для графического \n\
интерфейса пользователя с реализацией как рендеринга, так и физических \n\
аспектов проекта, в ней есть простое физическое приложение, \n\
демонстрирующее взаимодействие сфер и плоскостей кубов.\n\n");

  ImGui::Text(u8"Этот проект разработан Боутифор АбдЕлХак в рамках проекта ВКР "
              u8"в КФУ.\n\n");

  ImGui::Text(
      u8"Исходный код этого проекта можно найти на моей странице на github: \n\
https://github.com/Abh29/PhysicsGraphicCPP \n\
Я приглашаю всех желающих читать, копировать, изменять и улучшать.\n\n");

  ImGui::End();
}

// void ft::Gui::showSideBar(const ft::Scene::pointer &scene) {
//
//   (void)scene;
//   ImGui::Begin("Sidebar");
//
//   // Object Inspector
//   if (ImGui::CollapsingHeader("Object Inspector")) {
//     static uint32_t selectedObjectIndex = 0;
//     ImGui::Text("Objects in Scene:");
//
//     for (uint32_t i = 0; i < scene->getObjects().size(); i++) {
//       if (ImGui::Selectable("object name", selectedObjectIndex == i)) {
//         selectedObjectIndex = i;
//       }
//     }
//
//     glm::vec3 v;
//     if (selectedObjectIndex < scene->getObjects().size()) {
//       auto &obj = scene->getObjects()[selectedObjectIndex];
//       ImGui::Text("Selected Object: %s", obj->getModel()->getPath().c_str());
//       ImGui::SliderFloat3("Position", glm::value_ptr(v), -100.0f, 100.0f);
//       ImGui::SliderFloat3("Rotation", glm::value_ptr(v), -180.0f, 180.0f);
//       ImGui::SliderFloat3("Scale", glm::value_ptr(v), 0.1f, 10.0f);
//     }
//   }
//
//   float fov = scene->getCamera()->getFov();
//   // Scene Controls
//   if (ImGui::CollapsingHeader("Scene Controls")) {
//     ImGui::Text("Camera Settings:");
//     ImGui::SliderFloat3("Camera Position",
//                         glm::value_ptr(scene->getCamera()->getEyePosition()),
//                         -100.0f, 100.0f);
//
//     ImGui::SliderFloat("Field of View", &fov, 30.0f, 120.0f);
//
//     ImGui::Text("Lighting Settings:");
//     ImGui::ColorEdit3("Ambient Light",
//                       glm::value_ptr(scene->getUBO().lightDirection));
//     // Add more lighting controls as needed
//   }
//
//   // Simulation Controls
//   if (ImGui::CollapsingHeader("Simulation Controls")) {
//     if (ImGui::Button("Play")) { /* Play simulation */
//     }
//     ImGui::SameLine();
//     if (ImGui::Button("Pause")) { /* Pause simulation */
//     }
//     ImGui::SameLine();
//     if (ImGui::Button("Reset")) { /* Reset simulation */
//     }
//     // Add more simulation controls as needed
//   }
//
//   // Performance Metrics
//   if (ImGui::CollapsingHeader("Performance Metrics")) {
//     ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
//     // Add more performance metrics as needed
//   }
//
//   // Debugging Tools
//   if (ImGui::CollapsingHeader("Debugging Tools")) {
//     static bool showBoundingBoxes = false;
//     ImGui::Checkbox("Show Bounding Boxes", &showBoundingBoxes);
//     // Add more debugging tools as needed
//   }
//
//   // Settings and Preferences
//   if (ImGui::CollapsingHeader("Settings and Preferences")) {
//     static bool darkTheme = true;
//     if (ImGui::Checkbox("Dark Theme", &darkTheme)) {
//       if (darkTheme) {
//         ImGui::StyleColorsDark();
//       } else {
//         ImGui::StyleColorsLight();
//       }
//     }
//     // Add more settings and preferences as needed
//   }
//
//   ImGui::End();
// }
