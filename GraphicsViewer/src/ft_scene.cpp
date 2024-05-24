#include "../includes/ft_scene.h"
#include <cstdint>
#include <functional>
#include <glm/ext/matrix_transform.hpp>
#include <glm/fwd.hpp>
#include <glm/gtc/quaternion.hpp>
#include <ktx.h>
#include <memory>
#include <nlohmann/detail/value_t.hpp>
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
  for (auto &obj : _objects) {
    auto &model = obj->getModel();
    if (model->hasMaterial() || model->hasFlag(ft::MODEL_HIDDEN_BIT))
      continue;
    model->bind(commandBuffer, index);
    model->draw(commandBuffer, pipeline);
  }
}

void ft::Scene::drawMinimalObjs(const CommandBuffer::pointer &commandBuffer,
                                const GraphicsPipeline::pointer &pipeline,
                                const MinimalRdrSys::pointer &system,
                                uint32_t index) {

  // bind the graphics pipeline
  vkCmdBindPipeline(commandBuffer->getVKCommandBuffer(),
                    VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->getVKPipeline());

  vkCmdBindDescriptorSets(
      commandBuffer->getVKCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS,
      system->getGraphicsPipeline()->getVKPipelineLayout(), 0, 1,
      &(system->getDescriptorSets()[index]->getVKDescriptorSet()), 0, nullptr);

  // vertex and index buffers
  for (auto &obj : _objects) {
    auto &model = obj->getModel();
    if ((!model->hasFlag(ft::MODEL_HAS_ULTRA_SIMPLE_BIT) &&
         !model->hasFlag(ft::MODEL_SELECTED_BIT)) ||
        model->hasFlag(ft::MODEL_HIDDEN_BIT | ft::MODEL_LINE_BIT |
                       ft::MODEL_POINT_BIT))
      continue;
    model->bind(commandBuffer, index);
    model->draw(commandBuffer, pipeline);
  }

  if (_ftGizmo) {
    if (_state.lastSelect) {
      auto &m = _state.lastSelect;
      _ftGizmo->bind(commandBuffer, index);
      if (_state.globalGizbo) {
        _ftGizmo->resetTransform().at(_state.lastSelect->getCentroid());
      } else {
        _ftGizmo->resetTransform()
            .at(_state.lastSelect->getCentroid())
            .setRotation(m->getState().rotation);
      }
      _ftGizmo->draw(commandBuffer, pipeline);
    }
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
  for (auto &obj : _objects) {
    auto &model = obj->getModel();
    if ((!model->hasFlag(ft::MODEL_SIMPLE_BIT) &&
         !model->hasFlag(ft::MODEL_SELECTED_BIT)) ||
        model->hasFlag(ft::MODEL_HIDDEN_BIT | ft::MODEL_LINE_BIT |
                       ft::MODEL_POINT_BIT))
      continue;
    model->bind(commandBuffer, index);
    model->draw(commandBuffer, pipeline);
  }
}

void ft::Scene::drawSimpleObjsWithOutline(
    const CommandBuffer::pointer &commandBuffer,
    const SimpleRdrSys::pointer &srs, const OutlineRdrSys::pointer &ors,
    uint32_t index) {

  vkCmdBindDescriptorSets(
      commandBuffer->getVKCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS,
      srs->getGraphicsPipeline()->getVKPipelineLayout(), 0, 1,
      &(srs->getDescriptorSets()[index]->getVKDescriptorSet()), 0, nullptr);

  bool toggle = true;

  for (auto &obj : _objects) {
    auto &model = obj->getModel();
    if ((!model->hasFlag(ft::MODEL_SIMPLE_BIT) &&
         !model->hasFlag(ft::MODEL_SELECTED_BIT)) ||
        model->hasFlag(ft::MODEL_HIDDEN_BIT | ft::MODEL_LINE_BIT |
                       ft::MODEL_POINT_BIT))
      continue;

    model->bind(commandBuffer, index);

    if (model->hasFlag(ft::MODEL_SELECTED_BIT)) {
      // bind the graphics pipeline
      vkCmdBindPipeline(commandBuffer->getVKCommandBuffer(),
                        VK_PIPELINE_BIND_POINT_GRAPHICS,
                        ors->getGraphicsPipeline()->getVKPipeline());
      toggle = true;
      model->draw(commandBuffer, ors->getGraphicsPipeline());
    }

    if (toggle) {
      // bind the graphics pipeline
      vkCmdBindPipeline(commandBuffer->getVKCommandBuffer(),
                        VK_PIPELINE_BIND_POINT_GRAPHICS,
                        srs->getGraphicsPipeline()->getVKPipeline());

      toggle = false;
    }

    // vertex and index buffers
    model->draw(commandBuffer, srs->getGraphicsPipeline());
  }

  if (_ftGizmo) {
    _ftGizmo->bind(commandBuffer, index);
    if (_state.lastSelect) {
      auto &m = _state.lastSelect;
      if (_state.globalGizbo) {
        _ftGizmo->resetTransform().at(_state.lastSelect->getCentroid());
      } else {
        _ftGizmo->resetTransform()
            .at(_state.lastSelect->getCentroid())
            .setRotation(m->getState().rotation);
      }
      _ftGizmo->draw(commandBuffer, srs->getGraphicsPipeline());
    }
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
  for (auto &obj : _objects) {
    auto &model = obj->getModel();
    if (!model->hasFlag(ft::MODEL_HAS_COLOR_TEXTURE_BIT) ||
        model->hasFlag(ft::MODEL_HAS_NORMAL_TEXTURE_BIT) ||
        model->hasFlag(ft::MODEL_HIDDEN_BIT | ft::MODEL_SELECTED_BIT |
                       ft::MODEL_LINE_BIT | ft::MODEL_POINT_BIT))
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
  for (auto &obj : _objects) {
    auto &model = obj->getModel();
    if (!model->hasFlag(ft::MODEL_HAS_COLOR_TEXTURE_BIT) ||
        !model->hasFlag(ft::MODEL_HAS_NORMAL_TEXTURE_BIT) ||
        model->hasFlag(ft::MODEL_HIDDEN_BIT | ft::MODEL_SELECTED_BIT |
                       ft::MODEL_LINE_BIT | ft::MODEL_POINT_BIT))
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
  for (auto &obj : _objects) {
    auto &model = obj->getModel();
    if (!model->hasFlag(ft::MODEL_HAS_CUBE_TEXTURE_BIT) ||
        model->hasFlag(ft::MODEL_HIDDEN_BIT))
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
  for (auto &obj : _objects) {
    auto &model = obj->getModel();
    if (model->hasFlag(ft::MODEL_HIDDEN_BIT) ||
        !model->hasFlag(ft::MODEL_SELECTABLE_BIT))
      continue;
    model->bind(commandBuffer, index);
    model->draw(commandBuffer, pipeline);
  }

  if (_ftGizmo) {
    if (_state.lastSelect) {
      auto &m = _state.lastSelect;
      _ftGizmo->bind(commandBuffer, index);
      if (_state.globalGizbo) {
        _ftGizmo->resetTransform().at(_state.lastSelect->getCentroid());
      } else {
        _ftGizmo->resetTransform()
            .at(_state.lastSelect->getCentroid())
            .setRotation(m->getState().rotation);
      }
      _ftGizmo->draw(commandBuffer, pipeline);
    }
  }
}

void ft::Scene::drawOulines(const ft::CommandBuffer::pointer &commandBuffer,
                            const ft::SimpleRdrSys::pointer &srdr,
                            const ft::OutlineRdrSys::pointer &ordr,
                            uint32_t index) {
  // bind the graphics pipeline
  vkCmdBindPipeline(commandBuffer->getVKCommandBuffer(),
                    VK_PIPELINE_BIND_POINT_GRAPHICS,
                    ordr->getGraphicsPipeline()->getVKPipeline());

  vkCmdBindDescriptorSets(
      commandBuffer->getVKCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS,
      ordr->getGraphicsPipeline()->getVKPipelineLayout(), 0, 1,
      &(srdr->getDescriptorSets()[index]->getVKDescriptorSet()), 0, nullptr);

  // vertex and index buffers
  for (auto &obj : _objects) {
    auto &model = obj->getModel();
    if (!model->hasFlag(ft::MODEL_SELECTED_BIT) ||
        model->hasFlag(ft::MODEL_HIDDEN_BIT))
      continue;
    model->bind(commandBuffer, index);
    model->draw(commandBuffer, ordr->getGraphicsPipeline());
  }
}

void ft::Scene::drawPointsTopology(
    const ft::CommandBuffer::pointer &commandBuffer,
    const ft::SimpleRdrSys::pointer &srdr, const ft::PointRdrSys::pointer &prdr,
    uint32_t index) {
  // bind the graphics pipeline
  vkCmdBindPipeline(commandBuffer->getVKCommandBuffer(),
                    VK_PIPELINE_BIND_POINT_GRAPHICS,
                    prdr->getGraphicsPipeline()->getVKPipeline());

  vkCmdBindDescriptorSets(
      commandBuffer->getVKCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS,
      prdr->getGraphicsPipeline()->getVKPipelineLayout(), 0, 1,
      &(srdr->getDescriptorSets()[index]->getVKDescriptorSet()), 0, nullptr);

  for (auto &obj : _objects) {
    auto &model = obj->getModel();
    if (!model->hasFlag(ft::MODEL_POINT_BIT) ||
        model->hasFlag(ft::MODEL_HIDDEN_BIT))
      continue;

    model->bind(commandBuffer, index);
    model->draw(commandBuffer, prdr->getGraphicsPipeline());
  }
}

void ft::Scene::drawLinesTopology(
    const ft::CommandBuffer::pointer &commandBuffer,
    const ft::SimpleRdrSys::pointer &srdr, const ft::LineRdrSys::pointer &lrdr,
    uint32_t index) {
  // bind the graphics pipeline
  vkCmdBindPipeline(commandBuffer->getVKCommandBuffer(),
                    VK_PIPELINE_BIND_POINT_GRAPHICS,
                    lrdr->getGraphicsPipeline()->getVKPipeline());

  vkCmdBindDescriptorSets(
      commandBuffer->getVKCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS,
      lrdr->getGraphicsPipeline()->getVKPipelineLayout(), 0, 1,
      &(srdr->getDescriptorSets()[index]->getVKDescriptorSet()), 0, nullptr);

  // vertex and index buffers
  for (auto &obj : _objects) {
    auto &model = obj->getModel();
    if (!model->hasFlag(ft::MODEL_LINE_BIT) ||
        model->hasFlag(ft::MODEL_HIDDEN_BIT))
      continue;
    model->bind(commandBuffer, index);
    model->draw(commandBuffer, lrdr->getGraphicsPipeline());
  }

  if (_state.lastSelect) {
    auto &m = _state.lastSelect;
    m->bind(commandBuffer, index);
    m->drawAABB(commandBuffer, lrdr->getGraphicsPipeline());
  }
}

void ft::Scene::drawNormals(const ft::CommandBuffer::pointer &commandBuffer,
                            const ft::SimpleRdrSys::pointer &srdr,
                            const ft::NormDebugRdrSys::pointer &nrdr,
                            uint32_t index) {
  vkCmdBindPipeline(commandBuffer->getVKCommandBuffer(),
                    VK_PIPELINE_BIND_POINT_GRAPHICS,
                    nrdr->getGraphicsPipeline()->getVKPipeline());

  vkCmdBindDescriptorSets(
      commandBuffer->getVKCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS,
      nrdr->getGraphicsPipeline()->getVKPipelineLayout(), 0, 1,
      &(srdr->getDescriptorSets()[index]->getVKDescriptorSet()), 0, nullptr);

  // vertex and index buffers
  for (auto &obj : _objects) {
    auto &model = obj->getModel();
    if (!model->hasFlag(ft::MODEL_HAS_NORMAL_DEBUG_BIT) ||
        model->hasFlag(ft::MODEL_HIDDEN_BIT))
      continue;
    model->bind(commandBuffer, index);
    model->draw(commandBuffer, nrdr->getGraphicsPipeline(),
                VK_SHADER_STAGE_GEOMETRY_BIT);
  }
}

// load

ft::SceneObject::pointer
ft::Scene::addModelFromObj(const std::string &objectPath,
                           const ft::ObjectState &data) {
  Model::pointer model = std::make_shared<Model>(
      _ftDevice, objectPath, _ftUniformBuffers.size(), data.loadOptions);
  model->setState(data);
  model->setFlags(model->getID(), ft::MODEL_SIMPLE_BIT);
  auto obj = std::make_shared<SceneObject>(model);
  _objects.push_back(obj);
  SceneNode n;
  n._inputFile = objectPath;
  n._models.push_back(model);
  n._type = SceneNodeType::OBJ_SIMPLE;
  if ((data.loadOptions & ft::LOAD_OPTION_NO_SAVE) == 0)
    _sceneGraph.push_back(n);
  return obj;
}

// load from gltf

std::vector<ft::SceneObject::pointer>
ft::Scene::addModelFromGltf(const std::string &gltfPath,
                            const ft::ObjectState &data) {

  tinygltf::Model gltfInput;
  tinygltf::TinyGLTF gltfContext;
  std::string error, warning;

  if (!gltfContext.LoadASCIIFromFile(&gltfInput, &error, &warning, gltfPath))
    throw std::runtime_error("Could not open GLTF file!\n" + error + "\n" +
                             warning);

  // load gltf scene
  tinygltf::Scene &scene = gltfInput.scenes[0];
  std::vector<SceneObject::pointer> out;
  SceneNode n;
  n._inputFile = gltfPath;
  n._type = SceneNodeType::GLTF_SIMPLE;
  for (int i : scene.nodes) {
    const tinygltf::Node node = gltfInput.nodes[i];
    auto model = std::make_shared<Model>(
        _ftDevice, gltfInput, node, _ftUniformBuffers.size(), data.loadOptions);
    if (model->empty())
      continue;
    model->setFlags(model->getID(), ft::MODEL_SIMPLE_BIT);
    model->setState(data);
    auto obj = std::make_shared<SceneObject>(model);
    out.push_back(obj);
    _objects.push_back(obj);
    n._models.push_back(model);
  }
  _sceneGraph.push_back(n);
  return out;
}

std::vector<ft::SceneObject::pointer> ft::Scene::addDoubleTexturedFromGltf(
    const std::string &gltfPath, const DescriptorPool::pointer &pool,
    const DescriptorSetLayout::pointer &layout, const ft::ObjectState &data) {

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
  std::vector<ft::SceneObject::pointer> out;
  SceneNode n;
  n._inputFile = gltfPath;
  n._type = SceneNodeType::GLTF_DOUBLE_TEX;
  for (int i : scene.nodes) {
    const tinygltf::Node node = gltfInput.nodes[i];
    Model::pointer model = std::make_shared<Model>(
        _ftDevice, gltfInput, node, _ftUniformBuffers.size(), data.loadOptions);
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
    auto obj = std::make_shared<SceneObject>(model);
    out.push_back(obj);
    _objects.push_back(obj);
    n._models.push_back(model);
  }

  _sceneGraph.push_back(n);
  return out;
}

std::vector<ft::SceneObject::pointer> ft::Scene::addSingleTexturedFromGltf(
    const std::string &gltfPath, const DescriptorPool::pointer &pool,
    const DescriptorSetLayout::pointer &layout, const ft::ObjectState &data) {

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
  std::vector<ft::SceneObject::pointer> out;
  SceneNode n;
  n._inputFile = gltfPath;
  n._type = SceneNodeType::GLTF_SINGLE_TEX;
  for (auto i : scene.nodes) {
    const tinygltf::Node node = gltfInput.nodes[i];
    Model::pointer model = std::make_shared<Model>(
        _ftDevice, gltfInput, node, _ftUniformBuffers.size(), data.loadOptions);
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
    auto obj = std::make_shared<SceneObject>(model);
    out.push_back(obj);
    _objects.push_back(obj);
    n._models.push_back(model);
  }

  _sceneGraph.push_back(n);
  return out;
}

ft::SceneObject::pointer ft::Scene::addCubeBox(
    const std::string &gltfModel, const std::string &ktxTexturePath,
    const DescriptorPool::pointer &pool,
    const DescriptorSetLayout::pointer &layout, const ft::ObjectState data) {

  tinygltf::Model gltfInput;
  tinygltf::TinyGLTF gltfContext;
  std::string error, warning;

  // load a texture
  _ftCubeTexture = std::make_shared<ft::Texture>(
      _ftDevice, ktxTexturePath, ft::Texture::FileType::FT_TEXTURE_KTX_CUBE);
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
  SceneObject::pointer obj;
  SceneNode n;
  n._type = SceneNodeType::SKY_BOX;
  n._inputFile = gltfModel;
  n._texturePath = ktxTexturePath;
  for (int i : scene.nodes) {
    const tinygltf::Node node = gltfInput.nodes[i];
    auto model = std::make_shared<Model>(
        _ftDevice, gltfInput, node, _ftUniformBuffers.size(),
        ft::LOAD_OPTION_NO_AABB | data.loadOptions);
    if (model->empty())
      continue;
    auto rootNode = model->getAllNodes()[0];
    rootNode->mesh[0].material = material;
    rootNode->state.flags |= ft::MODEL_HAS_CUBE_TEXTURE_BIT;
    model->unsetFlags(model->getID(), ft::MODEL_SIMPLE_BIT);
    model->setState(data);
    obj = std::make_shared<SceneObject>(model);
    _objects.push_back(obj);
    n._models.push_back(model);
    break;
  }

  std::cout << "skybox loaded" << std::endl;
  _sceneGraph.push_back(n);
  _state.hasSkyBox = true;
  return obj;
}

// gizmo

ft::Gizmo::pointer ft::Scene::loadGizmo(const std::string &gltfPath) {
  _ftGizmo = std::make_shared<ft::Gizmo>(_ftDevice, gltfPath,
                                         _ftUniformBuffers.size());
  _ftGizmo->resetTransform();
  _ftGizmo->unselect();
  return _ftGizmo;
}

ft::Gizmo::pointer ft::Scene::getGizmo() const { return _ftGizmo; }

bool ft::Scene::hasGizmo() const { return _ftGizmo != nullptr; }

// set

void ft::Scene::addPointLightToTheScene(PointLightObject &pl) {
  _ubo.lights[_ubo.pLCount++] = pl;
}

ft::Camera::pointer ft::Scene::getCamera() const {
  if (_cameras.empty())
    return nullptr;
  return _cameras[_currentCamera];
}

void ft::Scene::addCamera(ft::Camera::pointer camera) {
  _cameras.push_back(std::move(camera));
  if (_cameras.size() == 1)
    _currentCamera = 0;
  _ubo.view = _cameras[_currentCamera]->getViewMatrix();
  _ubo.proj = _cameras[_currentCamera]->getProjMatrix();
  _ubo.eyePosition = _cameras[_currentCamera]->getEyePosition();
}

void ft::Scene::removeCurrentCamera() {
  if (_cameras.size() <= 1 || _cameras.size() <= _currentCamera)
    return;

  _cameras.erase(_cameras.begin() + _currentCamera);
  nextCamera();
}

void ft::Scene::nextCamera() {
  _currentCamera = (_currentCamera + 1) % _cameras.size();
  updateCameraUBO();
}

std::vector<ft::Camera::pointer> &ft::Scene::getAllCameras() {
  return _cameras;
}

bool ft::Scene::hasCamera() const { return !_cameras.empty(); }

void ft::Scene::setGeneralLight(glm::vec3 color, glm::vec3 direction,
                                float ambient) {
  _ubo.lightColor = color;
  _ubo.lightDirection = direction;
  _ubo.ambient = ambient;
}

ft::UniformBufferObject &ft::Scene::getUBO() { return _ubo; }

void ft::Scene::updateCameraUBO() {
  if (_cameras.empty())
    return;
  _ubo.view = _cameras[_currentCamera]->getViewMatrix();
  _ubo.proj = _cameras[_currentCamera]->getProjMatrix();
  _ubo.eyePosition = _cameras[_currentCamera]->getEyePosition();
  for (auto &buffer : _ftUniformBuffers)
    buffer->copyToMappedData(&_ubo, sizeof(_ubo));
}

ft::PointLightObject *ft::Scene::getLights() { return _ubo.lights; }

glm::vec3 &ft::Scene::getLightColor() { return _ubo.lightColor; }
glm::vec3 &ft::Scene::getLightPosition() { return _ubo.lightDirection; }

std::vector<ft::SceneObject::pointer> &ft::Scene::getObjects() {
  return _objects;
}

void ft::Scene::addMaterialToObj(uint32_t id, Material::pointer material) {
  for (auto &obj : _objects) {
    auto &m = obj->getModel();
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
  for (auto &obj : _objects) {
    auto &m = obj->getModel();
    if (m->findID(id)) {
      if (!m->hasFlag(ft::MODEL_SELECTABLE_BIT))
        return false;
      if (m->hasFlag(ft::MODEL_SELECTED_BIT)) {
        m->unsetFlags(m->getID(), ft::MODEL_SELECTED_BIT);
        _state.lastSelect = nullptr;
      } else {
        m->setFlags(m->getID(), ft::MODEL_SELECTED_BIT);
        if (_state.lastSelect)
          _state.lastSelect->unselectAll();
        _state.lastSelect = m.get();
      }
      return true;
    }
  }
  return false;
}

void ft::Scene::unselectAll() {
  for (auto &obj : _objects) {
    obj->getModel()->unselectAll();
  }
  _state.lastSelect = nullptr;
}

void ft::Scene::hideSelected() {
  if (_state.lastSelect) {
    _state.lastSelect->setFlags(_state.lastSelect->getID(),
                                ft::MODEL_HIDDEN_BIT);
    _state.lastSelect = nullptr;
  }
}

void ft::Scene::unhideSelected() {
  for (auto &obj : _objects)
    if (obj->getModel()->hasFlag(ft::MODEL_SELECTED_BIT))
      obj->getModel()->unsetFlags(obj->getModel()->getID(),
                                  ft::MODEL_HIDDEN_BIT);
}

void ft::Scene::unhideAll() {
  for (auto &obj : _objects)
    obj->getModel()->unsetFlags(obj->getModel()->getID(), ft::MODEL_HIDDEN_BIT);
}
void ft::Scene::resetAll() {
  for (auto &obj : _objects) {
    auto &m = obj->getModel();
    m->unsetFlags(m->getID(), ft::MODEL_HIDDEN_BIT);
    m->unsetFlags(m->getID(), ft::MODEL_LINE_BIT);
    m->unsetFlags(m->getID(), ft::MODEL_POINT_BIT);
    m->unsetFlags(m->getID(), ft::MODEL_SELECTED_BIT);
  }
}

void ft::Scene::toggleLinesTopo() {
  for (auto &obj : _objects) {
    auto &m = obj->getModel();
    if (m->hasFlag(ft::MODEL_SELECTED_BIT)) {
      m->toggleFlags(m->getID(), ft::MODEL_LINE_BIT);
      m->unsetFlags(m->getID(), ft::MODEL_POINT_BIT);
      m->unsetFlags(m->getID(), ft::MODEL_SELECTED_BIT);
    }
  }
}

void ft::Scene::togglePointsTopo() {
  for (auto &obj : _objects) {
    auto &m = obj->getModel();
    if (m->hasFlag(ft::MODEL_SELECTED_BIT)) {
      m->toggleFlags(m->getID(), ft::MODEL_POINT_BIT);
      m->unsetFlags(m->getID(), ft::MODEL_LINE_BIT);
      m->unsetFlags(m->getID(), ft::MODEL_SELECTED_BIT);
    }
  }
}

void ft::Scene::calculateNormals() {
  for (auto &obj : _objects) {
    auto &m = obj->getModel();
    if (m->hasFlag(ft::MODEL_SELECTED_BIT)) {
      m->reshade();
      m->updateVertexBuffer();
      m->unsetFlags(m->getID(), ft::MODEL_SELECTED_BIT);
      return;
    }
  }
}

void ft::Scene::toggleNormalDebug() {

  if (_state.lastSelect) {
    _state.lastSelect->toggleFlags(_state.lastSelect->getID(),
                                   ft::MODEL_HAS_NORMAL_DEBUG_BIT);
  }
}

void ft::Scene::showSelectedInfo() const {
  std::cout << "cameras: " << _cameras.size() << "\n";
  std::cout << "\tposition: " << glm::to_string(getCamera()->getEyePosition())
            << "\n";
  _ftGizmo->printInfo();
  std::cout << "\t" << (_state.globalGizbo ? "global" : "local") << "\n";
  if (_state.lastSelect) {
    auto &m = _state.lastSelect;
    std::cout << "model " << m->getID() << ": "
              << glm::to_string(m->getCentroid()) << "\n";
    std::cout << "\tvetices: " << _state.lastSelect->getVertices().size()
              << "\n";
    std::cout << "\tindices: " << _state.lastSelect->getIndices().size()
              << "\n";

    std::cout << "\tnodes: " << m->getAllNodes().size() << std::endl;

    std::cout << "\taabb: \n"
              << "\t\tmin: " << glm::to_string(m->getAABB().first) << "\n"
              << "\t\tmax: " << glm::to_string(m->getAABB().second)
              << std::endl;

    std::cout << "\ttransform:" << "\n";
    std::cout << "\t\tscale:\n";
    for (uint32_t i = 0; i < 4; ++i)
      std::cout << "\t\t " << glm::to_string(m->getState().scaling[i]) << "\n";

    std::cout << "\t\trotation:\n";
    for (uint32_t i = 0; i < 4; ++i)
      std::cout << "\t\t " << glm::to_string(m->getState().rotation[i]) << "\n";

    std::cout << "\t\ttransition:\n";
    for (uint32_t i = 0; i < 4; ++i)
      std::cout << "\t\t " << glm::to_string(m->getState().translation[i])
                << "\n";

    std::cout << "\tstate:\n";
    std::cout << "\t\tbasicColor: " << glm::to_string(m->getState().color)
              << "\n";
    std::cout << "\t\tloadOptions: " << m->getState().loadOptions << "\n";

    std::cout << "model: " << glm::to_string(m->getRootModelMatrix()) << "\n";
  }
}

void ft::Scene::toggleGizmo() { _state.globalGizbo = !_state.globalGizbo; }

bool ft::Scene::isGlobalGizmo() const { return _state.globalGizbo; }

ft::Model::raw_ptr ft::Scene::getSelectedModel() const {
  return _state.lastSelect;
}

bool ft::Scene::hasSkyBox() const { return _state.hasSkyBox; }

std::vector<ft::Scene::SceneNode> &ft::Scene::getSceneGraph() {
  return _sceneGraph;
}

// todo: this can be made concurent !
void ft::Scene::updateSceneObjects(float duration,
                                   ft::ThreadPool::pointer &pool) {

  std::vector<std::future<void>> tasks;
  for (auto &obj : _objects) {
    tasks.push_back(
        pool->addTask(std::bind(&SceneObject::update, *obj, duration)));
  }

  // wait for all tasks
  for (auto &t : tasks)
    t.wait();
}
