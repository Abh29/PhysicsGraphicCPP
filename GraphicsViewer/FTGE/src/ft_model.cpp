#include "../includes/ft_model.h"

ft::Model::Model(Device::pointer device, std::string filePath,
                 uint32_t bufferCount)
    : _ftDevice(device), _modelPath(filePath) {
  loadModel();
  createVertexBuffer();
  createIndexBuffer();

  _node->state.flags |= ft::MODEL_SIMPLE_BIT;
  BufferBuilder bufferBuilder;
  for (uint32_t i = 0; i < bufferCount; ++i) {
    _ftInstanceBuffers.push_back(
        bufferBuilder
            .setSize(sizeof(ft::PointLightObject) * ft::POINT_LIGHT_MAX_COUNT)
            .setUsageFlags(VK_BUFFER_USAGE_TRANSFER_DST_BIT |
                           VK_BUFFER_USAGE_VERTEX_BUFFER_BIT)
            .setMemoryProperties(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                 VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)
            .setIsMapped(true)
            .setMappedOffset(0)
            .setMappedFlags(0)
            .build(_ftDevice));
  }
}

ft::Model::Model(Device::pointer device, const tinygltf::Model &gltfInput,
                 const tinygltf::Node &inputNode, uint32_t bufferCount)
    : _ftDevice(std::move(device)) {

  loadNode(inputNode, gltfInput, nullptr);
  if (_vertices.empty()) {
    _node->state.flags |= ft::MODEL_IS_EMPTY_BIT;
  } else {
    createVertexBuffer();
    createIndexBuffer();
    if (_node->children.empty())
      _node->state.flags |= ft::MODEL_SIMPLE_BIT;
    BufferBuilder bufferBuilder;
    for (uint32_t i = 0; i < bufferCount; ++i) {
      _ftInstanceBuffers.push_back(
          bufferBuilder
              .setSize(sizeof(ft::PointLightObject) * ft::POINT_LIGHT_MAX_COUNT)
              .setUsageFlags(VK_BUFFER_USAGE_TRANSFER_DST_BIT |
                             VK_BUFFER_USAGE_VERTEX_BUFFER_BIT)
              .setMemoryProperties(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                   VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)
              .setIsMapped(true)
              .setMappedOffset(0)
              .setMappedFlags(0)
              .build(_ftDevice));
    }
  }
}

ft::Model::~Model() { delete _node; }

// todo: create a clone object
ft::Model::pointer ft::Model::clone() const {
  ft::Model *model = new Model();
  model->_ftDevice = _ftDevice;
  return std::shared_ptr<Model>(model);
}

void ft::Model::setState(const ft::InstanceData &data) {
  _node->state.modelMatrix = data.model;
  _node->state.normalMatrix = data.normalMatrix;
  _node->state.baseColor = data.color;
}

std::vector<ft::Model::Node *> ft::Model::getAllNodes() const {
  std::vector<ft::Model::Node *> out;
  out.reserve(_allNodes.size());
  for (auto i = _allNodes.begin(); i != _allNodes.end(); ++i)
    out.push_back(i->second);
  return out;
}

void ft::Model::bind(const ft::CommandBuffer::pointer &commandBuffer,
                     uint32_t index) {
  (void)index;
  VkBuffer vertexBuffers[] = {_ftVertexBuffer->getVKBuffer()};
  VkDeviceSize offsets[] = {0};
  vkCmdBindVertexBuffers(commandBuffer->getVKCommandBuffer(), 0, 1,
                         vertexBuffers, offsets);
  vkCmdBindIndexBuffer(commandBuffer->getVKCommandBuffer(),
                       _ftIndexBuffer->getVKBuffer(), 0, VK_INDEX_TYPE_UINT32);
}

void ft::Model::draw(const ft::CommandBuffer::pointer &commandBuffer,
                     const GraphicsPipeline::pointer &pipeline) {

  drawNode(commandBuffer, _node, pipeline, ignored);
  //    if (_node->state.flags & ft::MODEL_HAS_INDICES_BIT) {
  //        drawNode(commandBuffer, _node, pipeline);
  //    } else {
  //        std::cout << "this call is called" << std::endl;
  //        PushConstantObject push{_node->state.modelMatrix, _node->state.id};
  //        vkCmdPushConstants(commandBuffer->getVKCommandBuffer(),
  //                           pipeline->getVKPipelineLayout(),
  //                           VK_SHADER_STAGE_VERTEX_BIT,
  //                           0, sizeof(PushConstantObject),
  //                           &push);
  //        vkCmdDraw(commandBuffer->getVKCommandBuffer(),
  //        static_cast<uint32_t>(_indices.size()), _node->state.instanceCount,
  //        0, 0);
  //    }
}

void ft::Model::draw_extended(
    const CommandBuffer::pointer &commandBuffer,
    const GraphicsPipeline::pointer &pipeline,
    const std::function<void(const Primitive &)> &fun) {
  drawNode(commandBuffer, _node, pipeline, fun);
  _node->state.updated = false;
}

void ft::Model::loadModel() {
  tinyobj::attrib_t attrib;
  std::vector<tinyobj::shape_t> shapes;
  std::vector<tinyobj::material_t> materials;
  std::string warn, err;

  if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err,
                        _modelPath.c_str())) {
    throw std::runtime_error("tinyobj: " + warn + err);
  }

  std::unordered_map<Vertex, uint32_t> uniqueVertices{};

  for (const auto &shape : shapes) {
    for (const auto &index : shape.mesh.indices) {
      Vertex vertex{};

      vertex.pos = {
          attrib.vertices[3 * index.vertex_index + 0],
          attrib.vertices[3 * index.vertex_index + 1],
          attrib.vertices[3 * index.vertex_index + 2],
      };

      if (index.normal_index >= 0)
        vertex.normal = {
            attrib.normals[3 * index.normal_index + 0],
            attrib.normals[3 * index.normal_index + 1],
            attrib.normals[3 * index.normal_index + 2],
        };

      if (index.texcoord_index >= 0)
        vertex.texCoord = {
            attrib.texcoords[2 * index.texcoord_index + 0],
            1.0f - attrib.texcoords[2 * index.texcoord_index + 1],
        };

      vertex.color = {1.0f, 1.0f, 1.0f};

      if (uniqueVertices.count(vertex) == 0) {
        uniqueVertices[vertex] = static_cast<uint32_t>(_vertices.size());
        _vertices.push_back(vertex);
      }

      _indices.push_back(uniqueVertices[vertex]);
    }
  }

  _node = new Node();
  _node->parent = nullptr;
  _node->mesh.push_back(
      {0, static_cast<uint32_t>(_indices.size()), 0, nullptr});
  _node->state.name = _modelPath;
  _node->state.id = ID();

  _allNodes.insert(std::make_pair(_node->state.id, _node));
}

void ft::Model::writePerInstanceData(uint32_t index) {
  _ftInstanceBuffers[index]->copyToMappedData(
      _instances.data.data(), sizeof(InstanceData) * _instances.data.size());
}

void ft::Model::createVertexBuffer() {
  // create a staging buffer with memory
  VkDeviceSize bufferSize = sizeof(_vertices[0]) * _vertices.size();
  BufferBuilder bufferBuilder;

  auto stagingBuffer =
      bufferBuilder.setSize(bufferSize)
          .setUsageFlags(VK_BUFFER_USAGE_TRANSFER_SRC_BIT)
          .setMemoryProperties(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                               VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)
          .setIsMapped(true)
          .build(_ftDevice);

  // fill the staging vertex buffer
  stagingBuffer->copyToMappedData(_vertices.data(), bufferSize);

  // create a dest buffer
  _ftVertexBuffer =
      bufferBuilder.setSize(bufferSize)
          .setUsageFlags(VK_BUFFER_USAGE_TRANSFER_DST_BIT |
                         VK_BUFFER_USAGE_VERTEX_BUFFER_BIT)
          .setMemoryProperties(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
          .build(_ftDevice);

  // copy the data
  stagingBuffer->copyToBuffer(_ftVertexBuffer, bufferSize);
}

void ft::Model::createIndexBuffer() {
  VkDeviceSize bufferSize = sizeof(_indices[0]) * _indices.size();
  BufferBuilder bufferBuilder;

  // create a staging buffer
  auto stagingBuffer =
      bufferBuilder.setSize(bufferSize)
          .setUsageFlags(VK_BUFFER_USAGE_TRANSFER_SRC_BIT)
          .setMemoryProperties(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                               VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)
          .setIsMapped(true)
          .build(_ftDevice);

  // write the data to the staging buffer
  stagingBuffer->copyToMappedData(_indices.data(), bufferSize);

  // create the index buffer
  _ftIndexBuffer = bufferBuilder.setSize(bufferSize)
                       .setUsageFlags(VK_BUFFER_USAGE_TRANSFER_DST_BIT |
                                      VK_BUFFER_USAGE_INDEX_BUFFER_BIT)
                       .setMemoryProperties(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
                       .setIsMapped(false)
                       .build(_ftDevice);

  // copy buffer
  stagingBuffer->copyToBuffer(_ftIndexBuffer, bufferSize);
}

void ft::Model::loadNode(const tinygltf::Node &inputNode,
                         const tinygltf::Model &input, Node *parent) {
  auto *node = new Node();
  node->state.name = inputNode.name;
  node->parent = parent;
  node->state.id = Model::ID();

  node->state.modelMatrix = glm::mat4(1.0f);
  if (inputNode.translation.size() == 3) {
    node->state.modelMatrix =
        glm::translate(node->state.modelMatrix,
                       glm::vec3(glm::make_vec3(inputNode.translation.data())));
  }
  if (inputNode.rotation.size() == 4) {
    glm::quat q = glm::make_quat(inputNode.rotation.data());
    node->state.modelMatrix *= glm::mat4(q);
  }
  if (inputNode.scale.size() == 3) {
    node->state.modelMatrix =
        glm::scale(node->state.modelMatrix,
                   glm::vec3(glm::make_vec3(inputNode.scale.data())));
  }
  if (inputNode.matrix.size() == 16) {
    node->state.modelMatrix = glm::make_mat4x4(inputNode.matrix.data());
  }

  // load children nodes
  if (inputNode.children.size() > 0) {
    for (size_t i = 0; i < inputNode.children.size(); ++i) {
      loadNode(input.nodes[inputNode.children[i]], input, node);
    }
  }

  // load mesh data
  if (inputNode.mesh > -1) {
    const tinygltf::Mesh mesh = input.meshes[inputNode.mesh];
    for (size_t i = 0; i < mesh.primitives.size(); ++i) {
      const auto &p = mesh.primitives[i];
      uint32_t firstIndex = static_cast<uint32_t>(_indices.size());
      uint32_t vertexStart = static_cast<uint32_t>(_vertices.size());
      uint32_t indexCount = 0;

      // vertices
      const float *positionBuffer = nullptr;
      const float *normalsBuffer = nullptr;
      const float *texCoordsBuffer = nullptr;
      const float *tangentsBuffer = nullptr;
      size_t vertexCount = 0;

      if (p.attributes.find("POSITION") != p.attributes.end()) {
        const tinygltf::Accessor &accessor =
            input.accessors[p.attributes.find("POSITION")->second];
        const tinygltf::BufferView &view =
            input.bufferViews[accessor.bufferView];
        positionBuffer = reinterpret_cast<const float *>(
            &(input.buffers[view.buffer]
                  .data[accessor.byteOffset + view.byteOffset]));
        vertexCount = accessor.count;
      }
      if (p.attributes.find("NORMAL") != p.attributes.end()) {
        const tinygltf::Accessor &accessor =
            input.accessors[p.attributes.find("NORMAL")->second];
        const tinygltf::BufferView &view =
            input.bufferViews[accessor.bufferView];
        normalsBuffer = reinterpret_cast<const float *>(
            &(input.buffers[view.buffer]
                  .data[accessor.byteOffset + view.byteOffset]));
      }
      if (p.attributes.find("TEXCOORD_0") != p.attributes.end()) {
        const tinygltf::Accessor &accessor =
            input.accessors[p.attributes.find("TEXCOORD_0")->second];
        const tinygltf::BufferView &view =
            input.bufferViews[accessor.bufferView];
        texCoordsBuffer = reinterpret_cast<const float *>(
            &(input.buffers[view.buffer]
                  .data[accessor.byteOffset + view.byteOffset]));
      }
      if (p.attributes.find("TANGENT") != p.attributes.end()) {
        const tinygltf::Accessor &accessor =
            input.accessors[p.attributes.find("TAGENT")->second];
        const tinygltf::BufferView &view =
            input.bufferViews[accessor.bufferView];
        tangentsBuffer = reinterpret_cast<const float *>(
            &(input.buffers[view.buffer]
                  .data[accessor.byteOffset + view.byteOffset]));
      }
      for (size_t j = 0; j < vertexCount; ++j) {
        Vertex v{};
        v.pos = glm::make_vec3(&positionBuffer[j * 3]);
        if (normalsBuffer)
          v.normal = glm::make_vec3(&normalsBuffer[j * 3]);
        if (texCoordsBuffer)
          v.texCoord = glm::make_vec2(&texCoordsBuffer[j * 2]);
        if (tangentsBuffer)
          v.tangent = glm::make_vec4(&tangentsBuffer[j * 4]);
        v.color = glm::vec3(1.0f);
        _vertices.push_back(v);
      }

      // indices
      const tinygltf::Accessor &accessor = input.accessors[p.indices];
      const tinygltf::BufferView &bufferView =
          input.bufferViews[accessor.bufferView];
      const tinygltf::Buffer &buffer = input.buffers[bufferView.buffer];

      indexCount += static_cast<uint32_t>(accessor.count);
      switch (accessor.componentType) {
      case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT: {
        auto buff = reinterpret_cast<const uint32_t *>(
            &buffer.data[accessor.byteOffset + bufferView.byteOffset]);
        for (size_t j = 0; j < accessor.count; ++j) {
          _indices.push_back(buff[j] + vertexStart);
        }
        break;
      }
      case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT: {
        auto buff = reinterpret_cast<const uint16_t *>(
            &buffer.data[accessor.byteOffset + bufferView.byteOffset]);
        for (size_t j = 0; j < accessor.count; ++j) {
          _indices.push_back(buff[j] + vertexStart);
        }
        break;
      }
      case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE: {
        auto buff = reinterpret_cast<const uint8_t *>(
            &buffer.data[accessor.byteOffset + bufferView.byteOffset]);
        for (size_t j = 0; j < accessor.count; ++j) {
          _indices.push_back(buff[j] + vertexStart);
        }
        break;
      }
      default:
        // todo: do better
        return;
      }

      Primitive primitive{};
      primitive.firstIndex = firstIndex;
      primitive.indexCount = indexCount;
      primitive.materialIndex = p.material;
      node->mesh.push_back(primitive);
    }
  }

  if (parent) {
    parent->children.push_back(node);
  } else {
    _node = node;
  }

  _allNodes.insert(std::make_pair(node->state.id, node));
}

uint32_t ft::Model::ID() {
  static uint32_t id = 1;
  return id++;
}

uint32_t ft::Model::getID() const { return _node->state.id; }

bool ft::Model::findID(uint32_t id) const {
  return _allNodes.find(id) != _allNodes.end();
}

bool ft::Model::select(uint32_t id) {
  if (_allNodes.find(id) == _allNodes.end())
    return false;
  _allNodes[id]->state.flags |= ft::MODEL_SELECTED_BIT;
  _node->state.updated = true;
  return true;
}

bool ft::Model::unselect(uint32_t id) {
  if (_allNodes.find(id) == _allNodes.end())
    return false;
  _allNodes[id]->state.flags &= ~ft::MODEL_SELECTED_BIT;
  _node->state.updated = true;
  return true;
}

bool ft::Model::isSelected() const {
  return _node->state.flags & ft::MODEL_SELECTED_BIT;
}

bool ft::Model::empty() const {
  return _node->state.flags & ft::MODEL_IS_EMPTY_BIT;
}

void ft::Model::selectAll() {
  for (const auto &n : _allNodes) {
    n.second->state.flags |= ft::MODEL_SELECTED_BIT;
    _node->state.updated = true;
  }
}

void ft::Model::unselectAll() {
  for (const auto &n : _allNodes) {
    n.second->state.flags |= ft::MODEL_SELECTED_BIT;
    _node->state.updated = true;
  }
}

bool ft::Model::overrideFlags(uint32_t id, uint32_t flags) {
  for (const auto &n : _allNodes) {
    if (n.second->state.id == id) {
      n.second->state.flags = flags;
      _node->state.updated = true;
      return true;
    }
  }
  return false;
}

bool ft::Model::setFlags(uint32_t id, uint32_t flags) {
  for (const auto &n : _allNodes) {
    if (n.second->state.id == id) {
      n.second->state.flags |= flags;
      _node->state.updated = true;
      return true;
    }
  }
  return false;
}

bool ft::Model::unsetFlags(uint32_t id, uint32_t flags) {
  for (const auto &n : _allNodes) {
    if (n.second->state.id == id) {
      n.second->state.flags &= ~flags;
      _node->state.updated = true;
      return true;
    }
  }
  return false;
}

bool ft::Model::hasFlag(uint32_t flag) const {
  return (_node->state.flags & flag);
}

bool ft::Model::hasNodeFlag(uint32_t id, uint32_t flag) const {
  for (const auto &n : _allNodes) {
    if (n.second->state.id == id) {
      return (n.second->state.flags & flag);
    }
  }
  return false;
}

bool ft::Model::hasMaterial() {
  return (_node->state.flags & ft::MODEL_HAS_COLOR_TEXTURE_BIT ||
          _node->state.flags & ft::MODEL_HAS_NORMAL_TEXTURE_BIT);
}

bool ft::Model::isUpdated() const { return _node->state.updated = true; }

void ft::Model::drawNode(const CommandBuffer::pointer &commandBuffer,
                         Node *node, const GraphicsPipeline::pointer &pipeline,
                         const std::function<void(const Primitive &)> &fun) {
  if (node->state.flags & ft::MODEL_HIDDEN_BIT)
    return;

  if (!node->state.updated) {
    PushConstantObject push{node->state.updatedMatrix, node->state.baseColor,
                            node->state.id};
    vkCmdPushConstants(
        commandBuffer->getVKCommandBuffer(), pipeline->getVKPipelineLayout(),
        VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(PushConstantObject), &push);
  } else {
    node->state.updatedMatrix = node->state.modelMatrix;
    Node *currentParent = node->parent;
    while (currentParent) {
      node->state.updatedMatrix =
          currentParent->state.modelMatrix * node->state.updatedMatrix;
      currentParent = currentParent->parent;
    }

    PushConstantObject push{node->state.updatedMatrix, node->state.baseColor,
                            node->state.id};
    vkCmdPushConstants(
        commandBuffer->getVKCommandBuffer(), pipeline->getVKPipelineLayout(),
        VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(PushConstantObject), &push);
    node->state.updated = false;
  }

  for (const auto &p : node->mesh) {
    if (p.indexCount > 0) {
      fun(p);
      vkCmdDrawIndexed(commandBuffer->getVKCommandBuffer(), p.indexCount, 1,
                       p.firstIndex, 0, 0);
    }
  }
  for (auto &n : node->children)
    drawNode(commandBuffer, n, pipeline, fun);
}

uint32_t ft::Model::addCopy(const ft::InstanceData &copyData) {
  if (!(_node->state.flags & ft::MODEL_SIMPLE_BIT))
    return 0;
  uint32_t id = ID();
  _instances.data.push_back(copyData);
  _instances.ids.push_back(id);
  _instances.flags.push_back(0u);
  _instances.data[_instances.data.size() - 1].id = id;
  _node->state.instanceCount++;
  _node->state.flags |= ft::MODEL_HAS_INSTANCES_BIT;
  return id;
}

void ft::Model::ignored(const ft::Model::Primitive &primitive) {
  (void)primitive;
}

void ft::Model::addMaterial(const Material::pointer &material) {
  _node->mesh[0].material = material;
  for (auto &n : _allNodes) {
    for (auto &p : n.second->mesh) {
      p.material = material;
    }
    n.second->state.flags |= ft::MODEL_HAS_COLOR_TEXTURE_BIT;
  }
}

glm::mat4 &ft::Model::getRootModelMatrix() { return _node->state.modelMatrix; }
