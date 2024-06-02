#include "../includes/ft_jsonParser.h"
#include "ft_defines.h"
#include <cstdint>
#include <fstream>
#include <glm/ext/matrix_transform.hpp>
#include <glm/fwd.hpp>
#include <glm/trigonometric.hpp>
#include <nlohmann/json_fwd.hpp>
#include <ostream>
#include <stdexcept>

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

  _loadedFile = filePath;

  if (jsonData.contains("cameras"))
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

  if (!ft::tools::fileExists(filePath))
    throw std::runtime_error("Could not open " + filePath +
                             ", make sure the file exists and is writable!");

  std::ofstream file(filePath);
  if (!file) {
    throw std::runtime_error(
        "Could not open " + filePath +
        " for reading, make sure the file exists and is writable!");
  }

  nlohmann::json jsonData = _ignored;

  // cameras
  for (auto &cam : scene->getAllCameras()) {

    auto eye = cam->getEyePosition();
    auto tar = cam->getTargetPosition();
    auto up = cam->getUpDirection();

    nlohmann::json camData;

    camData["eyePosition"] = {eye.x, eye.y, eye.z};
    camData["target"] = {tar.x, tar.y, tar.z};
    camData["upDirection"] = {up.x, up.y, up.z};
    camData["fov"] = cam->getFov();
    camData["near"] = cam->getNearZ();
    camData["far"] = cam->getFarZ();

    jsonData["cameras"].push_back(camData);
  }

  // lights
  auto lc = scene->getUBO().lightColor;
  auto ld = scene->getUBO().lightDirection;

  jsonData["light"]["color"] = {lc.r, lc.g, lc.b};
  jsonData["light"]["direction"] = {ld.x, ld.y, ld.z};
  jsonData["light"]["ambient"] = scene->getUBO().ambient;

  for (uint32_t i = 0; i < scene->getUBO().pLCount; ++i) {
    auto &l = scene->getUBO().lights[i];
    nlohmann::json pl;

    pl["position"] = {l.position.x, l.position.y, l.position.z};
    pl["color"] = {l.color.x, l.color.y, l.color.z};
    pl["attenuation"] = {l.attenuation.x, l.attenuation.y, l.attenuation.z};
    pl["ambient"] = l.ambient;
    pl["diffuse"] = l.diffuse;
    pl["specular"] = l.specular;
    pl["on"] = static_cast<bool>(l.on);

    jsonData["light"]["pointLights"].push_back(pl);
  }

  // skybox
  auto &g = scene->getSceneGraph();
  if (scene->hasSkyBox()) {
    Scene::SceneNode sn;
    for (auto &n : g) {
      if (n._type == Scene::SceneNodeType::SKY_BOX) {
        sn = n;
        break;
      }
    }

    jsonData["skyBox"]["path"] = sn._inputFile;
    jsonData["skyBox"]["texture"] = sn._texturePath;
    jsonData["skyBox"]["ignore"] = false;
    auto &state = sn._models[0]->getState();
    jsonData["skyBox"]["basicColor"] = {state.color.r, state.color.g,
                                        state.color.b};
    auto m = sn._models[0]->getState().scaling;
    jsonData["skyBox"]["scaling"] = {m[0][0], m[1][1], m[2][2]};
  }

  // models
  nlohmann::json jmodels;
  if (_ignored.contains("models")) {
    jsonData["models"] = _ignored["models"];
  }

  for (auto &n : g) {
    nlohmann::json model;

    switch (n._type) {
    case ft::Scene::SceneNodeType::OBJ_SIMPLE:
      model["type"] = "obj";
      break;
    case ft::Scene::SceneNodeType::GLTF_SIMPLE:
    case ft::Scene::SceneNodeType::GLTF_SINGLE_TEX:
    case ft::Scene::SceneNodeType::GLTF_DOUBLE_TEX:
      model["type"] = "gltf";
      break;
    default:
      continue;
    };

    model["name"] = n._inputFile.substr(n._inputFile.find_last_of('/') + 1);
    model["path"] = n._inputFile;
    model["ignore"] = false;
    auto color = n._models[0]->getState().color;
    model["basicColor"] = {color.r, color.g, color.b};
    model["loadOptions"] = n._models[0]->getState().loadOptions;

    auto rot =
        ft::tools::getAngleAxisFromRotation(n._models[0]->getState().rotation);
    auto scl = n._models[0]->getState().scaling;
    auto trs = n._models[0]->getState().translation;

    model["transformation"]["scaling"] = {scl[0][0], scl[1][1], scl[2][2]};
    model["transformation"]["translation"] = {trs[3][0], trs[3][1], trs[3][2]};
    model["transformation"]["rotation"] = {
        glm::degrees(rot.first), rot.second.x, rot.second.y, rot.second.z};

    if (n._type == ft::Scene::SceneNodeType::OBJ_SIMPLE &&
        n._models[0]->hasFlag(ft::MODEL_HAS_COLOR_TEXTURE_BIT)) {
      model["texturePath"] = n._models[0]
                                 ->getAllNodes()[0]
                                 ->mesh[0]
                                 .material->getTexture(0)
                                 ->getTexturePath();
    }

    model["flags"]["selectable"] =
        n._models[0]->hasFlag(ft::MODEL_SELECTABLE_BIT);
    model["flags"]["hidden"] = n._models[0]->hasFlag(ft::MODEL_HIDDEN_BIT);
    model["flags"]["simple"] = n._models[0]->hasFlag(ft::MODEL_SIMPLE_BIT);
    model["flags"]["hasTexture"] =
        n._models[0]->hasFlag(ft::MODEL_HAS_COLOR_TEXTURE_BIT);
    model["flags"]["hasNormalTexture"] =
        n._models[0]->hasFlag(ft::MODEL_HAS_NORMAL_TEXTURE_BIT);
    model["flags"]["hasNormalDebug"] =
        n._models[0]->hasFlag(ft::MODEL_HAS_NORMAL_DEBUG_BIT);
    model["flags"]["hasLineTopology"] =
        n._models[0]->hasFlag(ft::MODEL_LINE_BIT);
    model["flags"]["hasPointTopology"] =
        n._models[0]->hasFlag(ft::MODEL_POINT_BIT);

    for (uint32_t i = 1; i < n._models.size(); ++i) {
      nlohmann::json submodel;
      auto &sm = n._models[i];
      auto &ms = sm->getState().scaling;
      auto &mt = sm->getState().translation;
      auto mr = ft::tools::getAngleAxisFromRotation(sm->getState().rotation);

      submodel["index"] = i;
      submodel["transformation"]["scaling"] = {ms[0][0], ms[1][1], ms[2][2]};
      submodel["transformation"]["translation"] = {mt[3][0], mt[3][1],
                                                   mt[3][2]};
      submodel["transformation"]["rotation"] = {mr.first, mr.second.x,
                                                mr.second.y, mr.second.z};

      auto mc = sm->getState().color;
      submodel["basicColor"] = {mc.r, mc.g, mc.b};
      submodel["loadOptions"] = sm->getState().loadOptions;

      submodel["flags"]["selectable"] = sm->hasFlag(ft::MODEL_SELECTABLE_BIT);
      submodel["flags"]["hidden"] = sm->hasFlag(ft::MODEL_HIDDEN_BIT);
      submodel["flags"]["simple"] = sm->hasFlag(ft::MODEL_SIMPLE_BIT);
      submodel["flags"]["hasTexture"] =
          sm->hasFlag(ft::MODEL_HAS_COLOR_TEXTURE_BIT);
      submodel["flags"]["hasNormalTexture"] =
          sm->hasFlag(ft::MODEL_HAS_NORMAL_TEXTURE_BIT);
      submodel["flags"]["hasNormalDebug"] =
          sm->hasFlag(ft::MODEL_HAS_NORMAL_DEBUG_BIT);
      submodel["flags"]["hasLineTopology"] = sm->hasFlag(ft::MODEL_LINE_BIT);
      submodel["flags"]["hasPointTopology"] = sm->hasFlag(ft::MODEL_POINT_BIT);

      model["subModels"].push_back(submodel);
    }

    jsonData["models"].push_back(model);
  }

  file << jsonData.dump(4);
  file.close();
}

void ft::JsonParser::loadCamera(const Scene::pointer &scene,
                                nlohmann::json &data, float aspect) {
  (void)scene;
  (void)data;
  std::cout << "loading cameras" << std::endl;

  for (auto &cam : data["cameras"]) {

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
    scene->addCamera(builder.build());
  }
}

void ft::JsonParser::loadSkyBox(const Scene::pointer &scene,
                                nlohmann::json &data) {
  (void)scene;
  (void)data;

  auto sky = data["skyBox"];
  ft::ObjectState s{};

  if (sky.contains("ignore") && (bool)sky["ignore"]) {
    _ignored["skyBox"] = sky;
    return;
  }

  std::cout << "loading skyBox" << std::endl;

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

static uint32_t getFlagsFromJsonSnippet(const nlohmann::json &snip) {
  uint32_t flags = 0;

  if (snip.contains("flags")) {
    auto fg = snip["flags"];

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

    if (fg.contains("hasPointTopology") && (bool)fg["hasPointTopology"])
      flags |= ft::MODEL_POINT_BIT;

    if (fg.contains("hasLineTopology") && (bool)fg["hasLineTopology"])
      flags |= ft::MODEL_LINE_BIT;
  }

  return flags;
}

static void getStateFromJsonSnippet(const nlohmann::json &snip,
                                    ft::ObjectState &state) {

  if (snip.contains("transformation")) {
    auto transform = snip["transformation"];

    if (transform.contains("translation")) {
      auto translation = snip["transformation"]["translation"];
      state.translation = glm::translate(
          glm::mat4(1.0f), {translation[0], translation[1], translation[2]});
    }

    if (transform.contains("rotation")) {
      auto rotation = snip["transformation"]["rotation"];
      state.rotation = glm::rotate(
          glm::mat4(1.0f), glm::radians((float)rotation[0]),
          {(float)rotation[1], (float)rotation[2], (float)rotation[3]});
    }

    if (transform.contains("scaling")) {
      auto scaling = snip["transformation"]["scaling"];
      state.scaling =
          glm::scale(glm::mat4(1.0f), {scaling[0], scaling[1], scaling[2]});
    }
  }

  if (snip.contains("basicColor"))
    state.color = {snip["basicColor"][0], snip["basicColor"][1],
                   snip["basicColor"][1]};
}

void ft::JsonParser::loadModels(const Scene::pointer &scene,
                                nlohmann::json &data) {

  for (const auto &model : data["models"]) {
    uint32_t flags = 0u;
    ft::ObjectState s{};

    if (model.contains("ignore") && (bool)model["ignore"]) {
      _ignored["models"].push_back(model);
      continue;
    }

    std::string modelName = model["name"];
    std::cout << "loading model: " << modelName << std::endl;
    std::string modelPath = model["path"];
    std::string fileType = model["type"];

    if (model.contains("loadOptions"))
      s.loadOptions = static_cast<uint32_t>(model["loadOptions"]);

    getStateFromJsonSnippet(model, s);
    flags = getFlagsFromJsonSnippet(model);

    if (fileType.compare("obj") == 0) {

      auto m = scene->addModelFromObj(modelPath, s)->getModel();
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

      std::vector<ft::SceneObject::pointer> m;

      if (flags & ft::MODEL_SIMPLE_BIT) {
        m = scene->addModelFromGltf(modelPath, s);
        for (const auto &i : m) {
          i->getModel()->setState(s);
          i->getModel()->setFlags(i->getModel()->getID(), flags);
        }

      } else if (flags & ft::MODEL_HAS_COLOR_TEXTURE_BIT &&
                 !(flags & ft::MODEL_HAS_NORMAL_TEXTURE_BIT)) {
        m = scene->addSingleTexturedFromGltf(
            modelPath, _ftTexturedRdrSys->getDescriptorPool(),
            _ftTexturedRdrSys->getDescriptorSetLayout(), s);

        for (const auto &i : m) {
          i->getModel()->setState(s);
          i->getModel()->setFlags(i->getModel()->getID(), flags);
        }

      } else if (flags & (ft::MODEL_HAS_COLOR_TEXTURE_BIT |
                          ft::MODEL_HAS_NORMAL_TEXTURE_BIT)) {

        m = scene->addDoubleTexturedFromGltf(
            modelPath, _ft2TexturedRdrSys->getDescriptorPool(),
            _ft2TexturedRdrSys->getDescriptorSetLayout(), s);

        for (const auto &i : m) {
          i->getModel()->setState(s);
          i->getModel()->setFlags(i->getModel()->getID(), flags);
        }
      }

      if (model.contains("subModels")) {
        for (auto &subM : model["subModels"]) {
          if (subM.contains("index") &&
              static_cast<uint32_t>(subM["index"]) < m.size()) {

            uint32_t index = subM["index"];

            if (subM.contains("transformation") ||
                subM.contains("basicColor")) {
              getStateFromJsonSnippet(subM, m[index]->getModel()->getState());
            }

            if (subM.contains("flags")) {
              auto subFlags = getFlagsFromJsonSnippet(subM);
              m[index]->getModel()->overrideFlags(m[index]->getModel()->getID(),
                                                  subFlags);
            }
          }
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

  if (light.contains("pointLights")) {
    auto &pls = light["pointLights"];
    for (auto &pl : pls) {
      ft::PointLightObject l = {};

      if (pl.contains("position"))
        l.position = {pl["position"][0], pl["position"][1], pl["position"][2]};

      if (pl.contains("color"))
        l.color = {pl["color"][0], pl["color"][1], pl["color"][2]};

      if (pl.contains("attenuation"))
        l.attenuation = {pl["attenuation"][0], pl["attenuation"][1],
                         pl["attenuation"][2]};

      if (pl.contains("ambient"))
        l.ambient = pl["ambient"];

      if (pl.contains("diffuse"))
        l.diffuse = pl["diffuse"];

      if (pl.contains("specular"))
        l.specular = pl["specular"];

      if (pl.contains("on"))
        l.on = static_cast<uint32_t>(pl["on"]);

      scene->addPointLightToTheScene(l);
    }
  }

  scene->setGeneralLight(color, direction, ambient);
}
