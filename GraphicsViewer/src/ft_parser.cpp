#include "../includes/ft_parser.h"
#include <cstdint>
#include <glm/ext/matrix_transform.hpp>
#include <glm/fwd.hpp>
#include <stdexcept>

ft::Parser::Parser(const Device::pointer &device,
                   const TexturePool::pointer &mPool,
                   const ThreadPool::pointer &tPool)
    : _ftDevice(device), _ftMaterialPool(mPool), _ftThreadPool(tPool) {}

/*********************************JsonParser**************************************/

ft::JsonParser::JsonParser(const Device::pointer &device,
                           const TexturePool::pointer &mPool,
                           const ThreadPool::pointer &tPool,
                           const OneTextureRdrSys::pointer &otrdr,
                           const TwoTextureRdrSys::pointer &ttrdr,
                           const SkyBoxRdrSys::pointer &sbrdr)
    : Parser(device, mPool, tPool), _ftTexturedRdrSys(otrdr),
      _ft2TexturedRdrSys(ttrdr), _ftSkyBoxRdrSys(sbrdr) {}

void ft::JsonParser::parseSceneFile(const ft::Scene::pointer &scene,
                                    const std::string &filePath, float aspect) {
  (void)scene;
  (void)filePath;

  if (!ft::tools::fileExists(filePath))
    throw std::runtime_error("Could not load " + filePath +
                             ", make sure the file exists and is readable!");

  std::ifstream file(filePath);

  // Read the entire file into a JSON object
  nlohmann::json jsonData;
  file >> jsonData;
  file.close();

  if (jsonData.contains("camera"))
    loadCamera(scene, jsonData, aspect);

  if (jsonData.contains("light"))
    loadLights(scene, jsonData);

  if (jsonData.contains("skyBox"))
    loadSkyBox(scene, jsonData);

  if (jsonData.contains("models"))
    loadModels(scene, jsonData);
}

void ft::JsonParser::saveSceneToFile(const ft::Scene::pointer &scene,
                                     const std::string &filePath) {
  (void)scene;
  (void)filePath;
  // TODO: implement this
}

void ft::JsonParser::loadCamera(const Scene::pointer &scene,
                                nlohmann::json &data, float aspect) {
  (void)scene;
  (void)data;
  std::cout << "loading camera" << std::endl;

  auto cam = data["camera"];

  CameraBuilder builder;

  if (cam.contains("eyePosition")) {
    auto eye = cam["eyePosition"];
    builder.setEyePosition({eye[0], eye[1], eye[2]});
  } else {
    builder.setEyePosition({5, -1, 0});
  }

  if (cam.contains("target")) {
    auto target = cam["target"];
    builder.setTarget({target[0], target[1], target[2]});
  } else {
    builder.setTarget({1.0, -1.0, 0.0});
  }

  if (cam.contains("upDirection")) {
    auto up = cam["upDirection"];
    builder.setUpDirection({up[0], up[1], up[2]});
  } else {
    builder.setUpDirection({0.0, 1.0, 0.0});
  }

  if (cam.contains("fov")) {
    builder.setFOV(cam["fov"]);
  } else {
    builder.setFOV(120.0f);
  }

  if (cam.contains("near") && cam.contains("far")) {
    builder.setZNearFar(cam["near"], cam["far"]);
  } else {
    builder.setZNearFar(0.5f, 1000.0f);
  }

  builder.setAspect(aspect);
  scene->setCamera(builder.build());
}

void ft::JsonParser::loadSkyBox(const Scene::pointer &scene,
                                nlohmann::json &data) {
  (void)scene;
  (void)data;
  std::cout << "loading skyBox" << std::endl;

  auto sky = data["skyBox"];
  ft::ObjectState s{};

  if (!sky.contains("path") || !sky.contains("texture"))
    throw std::runtime_error("a skybox must have a model path and texture !");

  if (sky.contains("basicColor"))
    s.color = {sky["basicColor"][0], sky["basicColor"][1],
               sky["basicColor"][2]};

  if (sky.contains("scaling")) {
    s.scaling = glm::scale(
        glm::mat4(1.0f),
        glm::vec3{sky["scaling"][0], sky["scaling"][1], sky["scaling"][2]});
  }

  scene->addCubeBox(sky["path"], sky["texture"],
                    _ftSkyBoxRdrSys->getDescriptorPool(),
                    _ftSkyBoxRdrSys->getDescriptorSetLayout(), s);
}
void ft::JsonParser::loadModels(const Scene::pointer &scene,
                                nlohmann::json &data) {

  for (const auto &model : data["models"]) {
    uint32_t flags = 0u;
    ft::ObjectState s{};

    if (model.contains("ignore") && (bool)model["ignore"])
      continue;

    std::string modelName = model["name"];
    std::cout << "loading model: " << modelName << std::endl;
    std::string modelPath = model["path"];
    std::string fileType = model["type"];

    if (model.contains("transformation")) {
      auto transform = model["transformation"];

      if (transform.contains("translation")) {
        auto translation = model["transformation"]["translation"];
        s.translation = glm::translate(
            glm::mat4(1.0f), {translation[0], translation[1], translation[2]});
      }

      if (transform.contains("rotation")) {
        auto rotation = model["transformation"]["rotation"];
        s.rotation =
            glm::rotate(glm::mat4(1.0f), glm::radians((float)rotation[0]),
                        {rotation[1], rotation[2], rotation[3]});
      }

      if (transform.contains("scaling")) {
        auto scaling = model["transformation"]["scaling"];
        s.scaling =
            glm::scale(glm::mat4(1.0f), {scaling[0], scaling[1], scaling[2]});
      }
    }

    if (model.contains("basicColor"))
      s.color = {model["basicColor"][0], model["basicColor"][1],
                 model["basicColor"][1]};

    if (model.contains("flags")) {
      auto fg = model["flags"];

      if (fg.contains("selectable") && (bool)fg["selectable"])
        flags |= ft::MODEL_SELECTABLE_BIT;

      if (fg.contains("hidden") && (bool)fg["hidden"])
        flags |= ft::MODEL_HIDDEN_BIT;

      if (fg.contains("simple") && (bool)fg["simple"])
        flags |= ft::MODEL_SIMPLE_BIT;

      if (fg.contains("hasTexture") && (bool)fg["hasTexture"])
        flags |= ft::MODEL_HAS_COLOR_TEXTURE_BIT;

      if (fg.contains("hasNormalTexture") && (bool)fg["hasNormalTexture"])
        flags |= ft::MODEL_HAS_NORMAL_TEXTURE_BIT;
    }

    if (fileType.compare("obj") == 0) {

      auto m = scene->addModelFromObj(modelPath, s);
      m->setFlags(m->getID(), flags);

      if (flags & ft::MODEL_HAS_COLOR_TEXTURE_BIT) {
        std::string texture = model["texturePath"];

        auto t = _ftMaterialPool->createTexture(
            texture, ft::Texture::FileType::FT_TEXTURE_PNG);
        auto material = std::make_shared<Material>(_ftDevice);
        material->addTexture(t);
        material->createDescriptors(
            _ftTexturedRdrSys->getDescriptorPool(),
            _ftTexturedRdrSys->getDescriptorSetLayout());
        for (int i = 0; i < ft::MAX_FRAMES_IN_FLIGHT; ++i) {
          material->bindDescriptor(i, 0, 1);
        }
        _ftMaterialPool->addMaterial(material);
        m->unsetFlags(m->getID(), ft::MODEL_SIMPLE_BIT);
        m->addMaterial(material);
        m->setFlags(m->getID(), ft::MODEL_HAS_COLOR_TEXTURE_BIT);
      }
    } else if (fileType.compare("gltf") == 0) {

      if (flags & ft::MODEL_SIMPLE_BIT) {
        auto m = scene->addModelFromGltf(modelPath, s);
        for (const auto &i : m)
          i->setFlags(i->getID(), flags);

      } else if (flags & ft::MODEL_HAS_COLOR_TEXTURE_BIT &&
                 !(flags & ft::MODEL_HAS_NORMAL_TEXTURE_BIT)) {
        auto m = scene->addSingleTexturedFromGltf(
            modelPath, _ftTexturedRdrSys->getDescriptorPool(),
            _ftTexturedRdrSys->getDescriptorSetLayout());

        for (const auto &i : m) {

          glm::mat4 &mat = i->getRootModelMatrix();
          mat = s.translation * s.rotation * s.scaling;
          i->setFlags(i->getID(), flags);
        }

      } else if (flags & (ft::MODEL_HAS_COLOR_TEXTURE_BIT |
                          ft::MODEL_HAS_NORMAL_TEXTURE_BIT)) {

        auto m = scene->addDoubleTexturedFromGltf(
            modelPath, _ft2TexturedRdrSys->getDescriptorPool(),
            _ft2TexturedRdrSys->getDescriptorSetLayout());

        for (const auto &i : m) {

          glm::mat4 &mat = i->getRootModelMatrix();
          mat = s.translation * s.rotation * s.scaling;
          i->setFlags(i->getID(), flags);
        }
      }
    }
  }
}

void ft::JsonParser::loadLights(const Scene::pointer &scene,
                                nlohmann::json &data) {
  (void)scene;
  (void)data;
  std::cout << "loading lights" << std::endl;

  glm::vec3 color = {1.0, 1.0, 1.0};
  glm::vec3 direction = {0.0, 2.5, 0.0};
  float ambient = 0.2f;

  auto light = data["light"];
  if (light.contains("color"))
    color = {light["color"][0], light["color"][1], light["color"][2]};

  if (light.contains("direction"))
    direction = {light["direction"][0], light["direction"][1],
                 light["direction"][2]};

  if (light.contains("ambient"))
    ambient = light["ambient"];

  scene->setGeneralLight(color, direction, ambient);
}
