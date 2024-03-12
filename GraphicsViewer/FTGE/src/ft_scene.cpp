#include "../includes/ft_scene.h"
#include <glm/fwd.hpp>
#include <ktx.h>
#include <memory>
#include <vulkan/vulkan_core.h>

ft::Scene::Scene(ft::Device::pointer device,
                 std::vector<ft::Buffer::pointer> ubos)
    : _ftDevice(std::move(device)), _ftUniformBuffers(std::move(ubos)) {

  // point lights
  _ubo.pLCount = 0;

  // general lighting
  _ubo.lightColor = {1.0f, 1.0f, 1.0f};
  _ubo.lightDirection = {10.0f, 10.0f, 10.0f};
  _ubo.ambient = 0.2f;
}

// draw

void ft::Scene::drawInstancedObjs(const CommandBuffer::pointer &commandBuffer,
                                  const GraphicsPipeline::pointer &pipeline,
                                  uint32_t index) {
  // bind the graphics pipeline
  vkCmdBindPipeline(commandBuffer->getVKCommandBuffer(),
                    VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->getVKPipeline());

  _ftUniformBuffers[index]->copyToMappedData(&_ubo, sizeof(_ubo));
  // vertex and index buffers
  for (auto &model : _models) {
    if (model->hasMaterial())
      continue;
    model->bind(commandBuffer, index);
    model->draw(commandBuffer, pipeline);
  }
}

void ft::Scene::drawSimpleObjs(const CommandBuffer::pointer &commandBuffer,
                               const GraphicsPipeline::pointer &pipeline,
                               const SimpleRdrSys::pointer &system,
                               uint32_t index) {

  // bind the graphics pipeline
  vkCmdBindPipeline(commandBuffer->getVKCommandBuffer(),
                    VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->getVKPipeline());

  vkCmdBindDescriptorSets(
      commandBuffer->getVKCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS,
      system->getGraphicsPipeline()->getVKPipelineLayout(), 0, 1,
      &(system->getDescriptorSets()[index]->getVKDescriptorSet()), 0, nullptr);

  // vertex and index buffers
  for (auto &model : _models) {
    if (!model->hasFlag(ft::MODEL_SIMPLE_BIT))
      continue;
    model->bind(commandBuffer, index);
    model->draw(commandBuffer, pipeline);
  }
}

void ft::Scene::drawTexturedObjs(
    const ft::CommandBuffer::pointer &commandBuffer,
    const ft::GraphicsPipeline::pointer &pipeline,
    const ft::OneTextureRdrSys::pointer &system, uint32_t index) {
  // bind the graphics pipeline
  vkCmdBindPipeline(commandBuffer->getVKCommandBuffer(),
                    VK_PIPELINE_BIND_POINT_GRAPHICS,
                    system->getGraphicsPipeline()->getVKPipeline());

  // vertex and index buffers
  for (auto &model : _models) {
    if (!model->hasFlag(ft::MODEL_HAS_COLOR_TEXTURE_BIT) ||
        model->hasFlag(ft::MODEL_HAS_NORMAL_TEXTURE_BIT))
      continue;
    model->bind(commandBuffer, index);
    model->draw_extended(
        commandBuffer, pipeline, [&](const Model::Primitive &p) {
          auto dsc = p.material->getDescriptorSet(index);
          // auto dsc = p.material->getTexture(0)->getDescriptorSet(index);
          dsc->updateDescriptorBuffer(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                                      _ftUniformBuffers[index], 0);
          vkCmdBindDescriptorSets(commandBuffer->getVKCommandBuffer(),
                                  VK_PIPELINE_BIND_POINT_GRAPHICS,
                                  pipeline->getVKPipelineLayout(), 0, 1,
                                  &(dsc->getVKDescriptorSet()), 0, nullptr);
        });
  }
}

void ft::Scene::draw2TexturedObjs(
    const ft::CommandBuffer::pointer &commandBuffer,
    const ft::GraphicsPipeline::pointer &pipeline,
    const ft::TwoTextureRdrSys::pointer &system, uint32_t index) {
  // bind the graphics pipeline
  vkCmdBindPipeline(commandBuffer->getVKCommandBuffer(),
                    VK_PIPELINE_BIND_POINT_GRAPHICS,
                    system->getGraphicsPipeline()->getVKPipeline());

  // vertex and index buffers
  for (auto &model : _models) {
    if (!model->hasFlag(ft::MODEL_HAS_COLOR_TEXTURE_BIT) ||
        !model->hasFlag(ft::MODEL_HAS_NORMAL_TEXTURE_BIT))
      continue;
    model->bind(commandBuffer, index);
    model->draw_extended(
        commandBuffer, pipeline, [&](const Model::Primitive &p) {
          auto dsc = p.material->getDescriptorSet(index);
          dsc->updateDescriptorBuffer(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                                      _ftUniformBuffers[index], 0);
          vkCmdBindDescriptorSets(commandBuffer->getVKCommandBuffer(),
                                  VK_PIPELINE_BIND_POINT_GRAPHICS,
                                  pipeline->getVKPipelineLayout(), 0, 1,
                                  &(dsc->getVKDescriptorSet()), 0, nullptr);
        });
  }
}

void ft::Scene::drawSkyBox(const ft::CommandBuffer::pointer &commandBuffer,
                           const ft::GraphicsPipeline::pointer &pipeline,
                           const ft::SkyBoxRdrSys::pointer &system,
                           uint32_t index) {
  // bind the graphics pipeline
  vkCmdBindPipeline(commandBuffer->getVKCommandBuffer(),
                    VK_PIPELINE_BIND_POINT_GRAPHICS,
                    system->getGraphicsPipeline()->getVKPipeline());

  // vertex and index buffers
  for (auto &model : _models) {
    if (!model->hasFlag(ft::MODEL_HAS_CUBE_TEXTURE_BIT))
      continue;
    model->bind(commandBuffer, index);
    model->draw_extended(
        commandBuffer, pipeline, [&](const Model::Primitive &p) {
          auto dsc = p.material->getDescriptorSet(index);
          dsc->updateDescriptorBuffer(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                                      _ftUniformBuffers[index], 0);
          vkCmdBindDescriptorSets(commandBuffer->getVKCommandBuffer(),
                                  VK_PIPELINE_BIND_POINT_GRAPHICS,
                                  pipeline->getVKPipelineLayout(), 0, 1,
                                  &(dsc->getVKDescriptorSet()), 0, nullptr);
        });
  }
}

void ft::Scene::drawPickObjs(const ft::CommandBuffer::pointer &commandBuffer,
                             const ft::GraphicsPipeline::pointer &pipeline,
                             uint32_t index) {
  // vertex and index buffers
  for (auto &model : _models) {
    model->bind(commandBuffer, index);
    model->draw(commandBuffer, pipeline);
  }
}

// load

ft::Model::pointer ft::Scene::addModelFromObj(const std::string &objectPath,
                                              ft::InstanceData data) {
  Model::pointer model =
      std::make_shared<Model>(_ftDevice, objectPath, _ftUniformBuffers.size());
  data.normalMatrix = glm::inverseTranspose(data.model * _ubo.view);
  data.id = model->getID();
  model->setState(data);
  model->setFlags(model->getID(), ft::MODEL_SIMPLE_BIT);
  _models.push_back(model);
  return model;
}

// load from gltf

ft::Model::pointer ft::Scene::addModelFromGltf(const std::string &gltfPath,
                                               ft::InstanceData data) {

  tinygltf::Model gltfInput;
  tinygltf::TinyGLTF gltfContext;
  std::string error, warning;

  if (!gltfContext.LoadASCIIFromFile(&gltfInput, &error, &warning, gltfPath))
    throw std::runtime_error("Could not open GLTF file!\n" + error + "\n" +
                             warning);

  // load gltf scene
  tinygltf::Scene &scene = gltfInput.scenes[0];
  Model::pointer model;
  for (int i : scene.nodes) {
    const tinygltf::Node node = gltfInput.nodes[i];
    model = std::make_shared<Model>(_ftDevice, gltfInput, node,
                                    _ftUniformBuffers.size());
    if (model->empty())
      continue;
    model->setFlags(model->getID(), ft::MODEL_SIMPLE_BIT);
    _models.push_back(model);
  }
  model->setState(data);
  return model;
}

std::vector<ft::Model::pointer> ft::Scene::addDoubleTexturedFromGltf(
    const std::string &gltfPath, const DescriptorPool::pointer &pool,
    const DescriptorSetLayout::pointer &layout) {

  tinygltf::Model gltfInput;
  tinygltf::TinyGLTF gltfContext;
  std::string error, warning;

  if (!gltfContext.LoadASCIIFromFile(&gltfInput, &error, &warning, gltfPath))
    throw std::runtime_error("Could not open GLTF file!\n" + error + "\n" +
                             warning);

  std::string path = gltfPath.substr(0, gltfPath.find_last_of('/'));

  std::vector<Texture::pointer> txts;
  // load images
  for (auto &image : gltfInput.images) {
    auto t = _ftTexturePool->createTexture(
        path + "/" + image.uri, ft::Texture::FileType::FT_TEXTURE_KTX);
    t->createDescriptorSets(layout, pool);
    txts.push_back(t);
  }

  // load materials
  std::vector<Material::pointer> materials;
  for (auto m : gltfInput.materials) {
    auto material = std::make_shared<Material>(_ftDevice);
    // base color factor
    if (m.values.find("baseColorFactor") != m.values.end()) {
      material->setColorFactor(
          glm::make_vec4(m.values["baseColorFactor"].ColorFactor().data()));
    }
    // base color texture index
    if (m.values.find("baseColorTexture") != m.values.end()) {
      uint32_t t = m.values["baseColorTexture"].TextureIndex();
      material->addTexture(txts[t]);
    } else {
      material->addTexture(txts[0]);
    }

    // normal map texture index
    if (m.additionalValues.find("normalTexture") != m.additionalValues.end()) {
      uint32_t t = m.additionalValues["normalTexture"].TextureIndex();
      material->addTexture(txts[t]);
    } else {
      material->addTexture(txts[0]);
    }

    material->setAlphaMode(m.alphaMode);
    material->setAlphaCutOff((float)m.alphaCutoff);
    material->setDoubleSided(m.doubleSided);
    material->createDescriptors(pool, layout);
    for (uint32_t j = 0; j < ft::MAX_FRAMES_IN_FLIGHT; ++j) {
      material->bindDescriptor(j, 0, 1);
      material->bindDescriptor(j, 1, 2);
    }
    _ftTexturePool->addMaterial(material);
    materials.push_back(material);
  }

  // load gltf scene
  tinygltf::Scene &scene = gltfInput.scenes[0];
  std::vector<ft::Model::pointer> out;
  for (int i : scene.nodes) {
    const tinygltf::Node node = gltfInput.nodes[i];
    Model::pointer model = std::make_shared<Model>(_ftDevice, gltfInput, node,
                                                   _ftUniformBuffers.size());
    if (model->empty())
      continue;
    auto allNodes = model->getAllNodes();
    for (auto &n : allNodes) {
      for (auto &p : n->mesh) {
        p.material = materials[p.materialIndex];
      }
      n->state.flags |=
          (ft::MODEL_HAS_NORMAL_TEXTURE_BIT | ft::MODEL_HAS_COLOR_TEXTURE_BIT);
    }
    model->unsetFlags(model->getID(), ft::MODEL_SIMPLE_BIT);
    out.push_back(model);
    _models.push_back(model);
  }

  return out;
}

std::vector<ft::Model::pointer> ft::Scene::addSingleTexturedFromGltf(
    const std::string &gltfPath, const DescriptorPool::pointer &pool,
    const DescriptorSetLayout::pointer &layout) {

  tinygltf::Model gltfInput;
  tinygltf::TinyGLTF gltfContext;
  std::string error, warning;

  if (!gltfContext.LoadASCIIFromFile(&gltfInput, &error, &warning, gltfPath))
    throw std::runtime_error("Could not open GLTF file!\n" + error + "\n" +
                             warning);

  std::string path = gltfPath.substr(0, gltfPath.find_last_of('/'));

  std::vector<Texture::pointer> txts;
  // load images
  for (auto &image : gltfInput.images) {
    auto t = _ftTexturePool->createTexture(
        path + "/" + image.uri, ft::Texture::FileType::FT_TEXTURE_PNG);
    txts.push_back(t);
  }

  // load materials
  std::vector<Material::pointer> materials;
  for (auto m : gltfInput.materials) {
    auto material = std::make_shared<Material>(_ftDevice);
    // base color factor
    if (m.values.find("baseColorFactor") != m.values.end()) {
      material->setColorFactor(
          glm::make_vec4(m.values["baseColorFactor"].ColorFactor().data()));
    }
    // base color texture index
    if (m.values.find("baseColorTexture") != m.values.end()) {
      uint32_t t = m.values["baseColorTexture"].TextureIndex();
      material->addTexture(txts[t]);
    } else {
      material->addTexture(txts[0]);
    }

    material->setAlphaMode(m.alphaMode);
    material->setAlphaCutOff((float)m.alphaCutoff);
    material->setDoubleSided(m.doubleSided);
    material->createDescriptors(pool, layout);
    for (uint32_t j = 0; j < ft::MAX_FRAMES_IN_FLIGHT; ++j) {
      material->bindDescriptor(j, 0, 1);
    }
    _ftTexturePool->addMaterial(material);
    materials.push_back(material);
  }

  // load gltf scene
  tinygltf::Scene &scene = gltfInput.scenes[0];
  std::vector<ft::Model::pointer> out;
  for (auto i : scene.nodes) {
    const tinygltf::Node node = gltfInput.nodes[i];
    Model::pointer model = std::make_shared<Model>(_ftDevice, gltfInput, node,
                                                   _ftUniformBuffers.size());
    if (model->empty())
      continue;
    auto allNodes = model->getAllNodes();
    for (auto &n : allNodes) {
      for (auto &p : n->mesh) {
        p.material = materials[p.materialIndex];
      }
      n->state.flags |= ft::MODEL_HAS_COLOR_TEXTURE_BIT;
    }
    model->unsetFlags(model->getID(), ft::MODEL_SIMPLE_BIT);
    out.push_back(model);
    _models.push_back(model);
  }

  return out;
}

ft::Model::pointer ft::Scene::addCubeBox(
    const std::string &gltfModel, const std::string &ktxTexture,
    const DescriptorPool::pointer &pool,
    const DescriptorSetLayout::pointer &layout, const ft::InstanceData data) {

  tinygltf::Model gltfInput;
  tinygltf::TinyGLTF gltfContext;
  std::string error, warning;

  // load a texture
  _ftCubeTexture = std::make_shared<ft::Texture>(
      _ftDevice, ktxTexture, ft::Texture::FileType::FT_TEXTURE_KTX_CUBE);
  _ftCubeTexture->createDescriptorSets(layout, pool);

  // create a material
  auto material = std::make_shared<Material>(_ftDevice);
  material->addTexture(_ftCubeTexture);
  material->createDescriptors(pool, layout);
  for (uint32_t j = 0; j < ft::MAX_FRAMES_IN_FLIGHT; ++j) {
    material->bindDescriptor(j, 0, 1);
  }
  _ftTexturePool->addMaterial(material);
  (void)gltfModel;
  (void)data;
  // load the model
  if (!gltfContext.LoadASCIIFromFile(&gltfInput, &error, &warning, gltfModel))
    throw std::runtime_error("Could not open GLTF file!\n" + error + "\n" +
                             warning);

  // load gltf scene
  tinygltf::Scene &scene = gltfInput.scenes[0];
  Model::pointer model;
  for (int i : scene.nodes) {
    const tinygltf::Node node = gltfInput.nodes[i];
    model = std::make_shared<Model>(_ftDevice, gltfInput, node,
                                    _ftUniformBuffers.size());
    if (model->empty())
      continue;
    auto rootNode = model->getAllNodes()[0];
    rootNode->mesh[0].material = material;
    rootNode->state.flags |= ft::MODEL_HAS_CUBE_TEXTURE_BIT;
    model->unsetFlags(model->getID(), ft::MODEL_SIMPLE_BIT);
    model->setState(data);
    _models.push_back(model);
    break;
  }

  return model;
}

// set

void ft::Scene::addPointLightToTheScene(PointLightObject &pl) {
  _ubo.lights[_ubo.pLCount++] = pl;
}

uint32_t ft::Scene::addObjectCopyToTheScene(uint32_t id,
                                            ft::InstanceData data) {
  data.normalMatrix = glm::inverseTranspose(data.model * _ubo.view);
  for (auto &m : _models) {
    if (m->findID(id))
      return m->addCopy(data);
  }
  return -1;
}

ft::Camera::pointer ft::Scene::getCamera() const { return _camera; }

void ft::Scene::setCamera(ft::Camera::pointer camera) {
  _camera = std::move(camera);
  _ubo.view = _camera->getViewMatrix();
  _ubo.proj = _camera->getProjMatrix();
  _ubo.eyePosition = _camera->getEyePosition();
}

void ft::Scene::setGeneralLight(glm::vec3 color, glm::vec3 direction,
                                float ambient) {
  _ubo.lightColor = color;
  _ubo.lightDirection = direction;
  _ubo.ambient = ambient;
}

void ft::Scene::updateCameraUBO() {
  _ubo.view = _camera->getViewMatrix();
  _ubo.proj = _camera->getProjMatrix();
  _ubo.eyePosition = _camera->getEyePosition();
  for (auto &buffer : _ftUniformBuffers)
    buffer->copyToMappedData(&_ubo, sizeof(_ubo));
}

ft::PointLightObject *ft::Scene::getLights() { return _ubo.lights; }

std::vector<ft::Model::pointer> ft::Scene::getModels() const { return _models; }

void ft::Scene::addMaterialToObj(uint32_t id, Material::pointer material) {
  for (auto &m : _models) {
    if (m->findID(id)) {
      m->addMaterial(std::move(material));
      break;
    }
  }
}

void ft::Scene::setMaterialPool(TexturePool::pointer pool) {
  _ftTexturePool = std::move(pool);
}

bool ft::Scene::select(uint32_t id) {
  for (auto &m : _models) {
    if (m->findID(id)) {
      return m->select(id);
    }
  }
  return false;
}

void ft::Scene::unselectAll() {
  for (auto &m : _models) {
    m->unselectAll();
  }
}
