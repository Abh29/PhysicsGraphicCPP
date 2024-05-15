#include "../includes/ft_model.h"
#include <algorithm>
#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/quaternion_geometric.hpp>
#include <glm/ext/scalar_constants.hpp>
#include <glm/ext/vector_relational.hpp>
#include <glm/fwd.hpp>
#include <glm/geometric.hpp>
#include <iostream>
#include <vulkan/vulkan_core.h>

ft::Model::Model(Device::pointer device, std::string filePath,
                 uint32_t bufferCount, uint32_t options)
    : _ftDevice(device), _modelPath(filePath), _centroid(), _oldCentroid() {

  _aabb.min = {std::numeric_limits<float>::max(),
               std::numeric_limits<float>::max(),
               std::numeric_limits<float>::max()};
  _aabb.max = {std::numeric_limits<float>::min(),
               std::numeric_limits<float>::min(),
               std::numeric_limits<float>::min()};

  _objectState.loadOptions = options;
  loadModel(options);
  _oldCentroid = _centroid;

  if ((options & ft::LOAD_OPTION_NO_AABB) == 0)
    createAABB();

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
                 const tinygltf::Node &inputNode, uint32_t bufferCount,
                 uint32_t options)
    : _ftDevice(std::move(device)), _centroid(), _oldCentroid() {

  _aabb.min = {std::numeric_limits<float>::max(),
               std::numeric_limits<float>::max(),
               std::numeric_limits<float>::max()};
  _aabb.max = {std::numeric_limits<float>::min(),
               std::numeric_limits<float>::min(),
               std::numeric_limits<float>::min()};

  _objectState.loadOptions = options;
  loadNode(inputNode, gltfInput, nullptr, options);
  _oldCentroid = _centroid;

  if ((options & ft::LOAD_OPTION_NO_AABB) == 0)
    createAABB();

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
                     const GraphicsPipeline::pointer &pipeline,
                     const VkShaderStageFlags stages) {

  drawNode(commandBuffer, _node, pipeline, ignored, stages);
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

void ft::Model::drawAABB(const ft::CommandBuffer::pointer &commandBuffer,
                         const GraphicsPipeline::pointer &pipeline,
                         const VkShaderStageFlags stages) {
  if (_aabb.flags & ft::MODEL_HIDDEN_BIT)
    return;

  // push constants
  PushConstantObject push{_node->state.modelMatrix, _aabb.color, 0u};
  vkCmdPushConstants(commandBuffer->getVKCommandBuffer(),
                     pipeline->getVKPipelineLayout(), stages, 0,
                     sizeof(PushConstantObject), &push);

  vkCmdDrawIndexed(commandBuffer->getVKCommandBuffer(), _aabb.indexCount, 1,
                   _aabb.firstIndex, 0, 0);
}

void ft::Model::draw_extended(const CommandBuffer::pointer &commandBuffer,
                              const GraphicsPipeline::pointer &pipeline,
                              const std::function<void(const Primitive &)> &fun,
                              const VkShaderStageFlags stages) {
  drawNode(commandBuffer, _node, pipeline, fun, stages);
  _node->state.updated = false;
}

void ft::Model::updateVertexBuffer() {
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

  // copy the data
  stagingBuffer->copyToBuffer(_ftVertexBuffer, bufferSize);
}

void ft::Model::reshade() {
  uint count = 0;
  for (auto &v : _vertices)
    v.normal = {};
  for (size_t i = 0; i < _indices.size(); i += 3) {
    glm::vec3 a = _vertices[_indices[i + 1]].pos - _vertices[_indices[i]].pos;
    glm::vec3 b = _vertices[_indices[i + 2]].pos - _vertices[_indices[i]].pos;
    if (glm::length2(_vertices[_indices[i]].normal) < 1e-6) {
      _vertices[_indices[i]].normal = glm::normalize(glm::cross(a, b));
    } else {
      _vertices[_indices[i]].normal =
          glm::normalize(_vertices[_indices[i]].normal + glm::cross(a, b));
    }
    ++count;

    a = _vertices[_indices[i + 2]].pos - _vertices[_indices[i + 1]].pos;
    b = _vertices[_indices[i]].pos - _vertices[_indices[i + 1]].pos;
    if (glm::length2(_vertices[_indices[i + 1]].normal) < 1e-6) {
      _vertices[_indices[i + 1]].normal = glm::normalize(glm::cross(a, b));
    } else {
      _vertices[_indices[i + 1]].normal =
          glm::normalize(_vertices[_indices[i + 1]].normal + glm::cross(a, b));
    }
    ++count;

    a = _vertices[_indices[i]].pos - _vertices[_indices[i + 2]].pos;
    b = _vertices[_indices[i + 1]].pos - _vertices[_indices[i + 2]].pos;
    if (glm::length2(_vertices[_indices[i + 2]].normal) < 1e-6) {
      _vertices[_indices[i + 2]].normal = glm::normalize(glm::cross(a, b));
    } else {
      _vertices[_indices[i + 2]].normal =
          glm::normalize(_vertices[_indices[i + 2]].normal + glm::cross(a, b));
    }
    ++count;
  }
  std::cout << "normals calculated " << count << std::endl;
}

void ft::Model::loadModel(uint32_t options) {
  tinyobj::attrib_t attrib;
  std::vector<tinyobj::shape_t> shapes;
  std::vector<tinyobj::material_t> materials;
  std::string warn, err;

  if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err,
                        _modelPath.c_str())) {
    throw std::runtime_error("tinyobj: " + warn + err);
  }

  glm::vec3 sign = {
      (options & ft::LOAD_OPTION_INVERSE_X) ? -1.0 : 1.0,
      (options & ft::LOAD_OPTION_INVERSE_Y) ? -1.0 : 1.0,
      (options & ft::LOAD_OPTION_INVERSE_Z) ? -1.0 : 1.0,
  };

  std::unordered_map<Vertex, uint32_t> uniqueVertices{};

  for (const auto &shape : shapes) {
    for (const auto &index : shape.mesh.indices) {
      Vertex vertex{};

      vertex.pos = {
          attrib.vertices[3 * index.vertex_index + 0],
          attrib.vertices[3 * index.vertex_index + 1],
          attrib.vertices[3 * index.vertex_index + 2],
      };

      vertex.pos *= sign;

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

        _centroid = _centroid +
                    (1 / (float)_vertices.size()) * (vertex.pos - _centroid);
        _aabb.min.x = std::min(_aabb.min.x, vertex.pos.x);
        _aabb.min.y = std::min(_aabb.min.y, vertex.pos.y);
        _aabb.min.z = std::min(_aabb.min.z, vertex.pos.z);
        _aabb.max.x = std::max(_aabb.max.x, vertex.pos.x);
        _aabb.max.y = std::max(_aabb.max.y, vertex.pos.y);
        _aabb.max.z = std::max(_aabb.max.z, vertex.pos.z);
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
                         const tinygltf::Model &input, Node *parent,
                         uint32_t options) {
  auto *node = new Node();
  node->state.name = inputNode.name;
  node->parent = parent;
  node->state.id = Model::ID();

  glm::vec3 sign = {
      (options & ft::LOAD_OPTION_INVERSE_X) ? -1.0 : 1.0,
      (options & ft::LOAD_OPTION_INVERSE_Y) ? -1.0 : 1.0,
      (options & ft::LOAD_OPTION_INVERSE_Z) ? -1.0 : 1.0,
  };

  node->state.modelMatrix = glm::mat4(1.0f);
  // add this in another manner

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
      loadNode(input.nodes[inputNode.children[i]], input, node, options);
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
        v.pos *= sign;
        if (normalsBuffer)
          v.normal = glm::make_vec3(&normalsBuffer[j * 3]);
        if (texCoordsBuffer)
          v.texCoord = glm::make_vec2(&texCoordsBuffer[j * 2]);
        if (tangentsBuffer)
          v.tangent = glm::make_vec4(&tangentsBuffer[j * 4]);
        v.color = glm::vec3(1.0f);

        _vertices.push_back(v);

        _centroid = _centroid + (1.0f / _vertices.size()) * (v.pos - _centroid);
        _aabb.min.x = std::min(_aabb.min.x, v.pos.x);
        _aabb.min.y = std::min(_aabb.min.y, v.pos.y);
        _aabb.min.z = std::min(_aabb.min.z, v.pos.z);
        _aabb.max.x = std::max(_aabb.max.x, v.pos.x);
        _aabb.max.y = std::max(_aabb.max.y, v.pos.y);
        _aabb.max.z = std::max(_aabb.max.z, v.pos.z);
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

void ft::Model::createAABB() {

  auto &min = _aabb.min;
  auto &max = _aabb.max;
  std::vector<glm::vec3> vertices = {
      {min.x, min.y, min.z}, // 0
      {max.x, min.y, min.z}, // 1
      {max.x, max.y, min.z}, // 2
      {min.x, max.y, min.z}, // 3
      {min.x, min.y, max.z}, // 4
      {max.x, min.y, max.z}, // 5
      {max.x, max.y, max.z}, // 6
      {min.x, max.y, max.z}  // 7
  };

  _aabb.firstIndex = _indices.size();
  std::vector<uint32_t> is = {
      // Bottom face
      0, 1, 1, 5, 5, 4, 4, 0, 0, 5, 1, 4, // Diagonals of the bottom face
      // Top face
      3, 2, 2, 6, 6, 7, 7, 3, 3, 6, 2, 7, // Diagonals of the top face
      // Right face
      1, 2, 2, 6, 6, 5, 5, 1, 1, 6, 2, 5, // Diagonals of the right face
      // Left face
      0, 3, 3, 7, 7, 4, 4, 0, 0, 7, 3, 4, // Diagonals of the left face
      // Front face
      0, 1, 1, 2, 2, 3, 3, 0, 0, 2, 1, 3, // Diagonals of the front face
      // Back face
      4, 5, 5, 6, 6, 7, 7, 4, 4, 6, 5, 7 // Diagonals of the back face
  };
  _aabb.indexCount = is.size();

  std::for_each(is.begin(), is.end(),
                [&](uint32_t &i) { i += _vertices.size(); });

  _indices.insert(_indices.end(), is.begin(), is.end());

  for (auto p : vertices) {
    Vertex v = {};
    v.pos = p;
    _vertices.push_back(v);
  }

  _aabb.color = {0.0f, 1.0f, 0.1f};
  createVertexBuffer();
  createIndexBuffer();
}

// flag manager

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
    n.second->state.flags &= ~ft::MODEL_SELECTED_BIT;
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

bool ft::Model::toggleFlags(uint32_t id, uint32_t flag) {
  for (const auto &n : _allNodes) {
    if (n.second->state.id == id) {
      n.second->state.flags ^= flag;
      return true;
    }
  }
  return false;
}

bool ft::Model::hasMaterial() {
  return (_node->state.flags & ft::MODEL_HAS_COLOR_TEXTURE_BIT ||
          _node->state.flags & ft::MODEL_HAS_NORMAL_TEXTURE_BIT);
}

bool ft::Model::isUpdated() const { return _node->state.updated = true; }

// state manager

void ft::Model::setState(const ft::ObjectState &data) {
  uint32_t t = _objectState.loadOptions;
  _objectState = data;
  _objectState.loadOptions = t;
  _node->state.modelMatrix = data.translation * data.rotation * data.scaling;
  _centroid = _node->state.modelMatrix * glm::vec4(_oldCentroid, 1.0f);
  for (auto &n : _allNodes)
    n.second->state.baseColor = data.color;
}

ft::ObjectState ft::Model::getState() const { return _objectState; }

ft::ObjectState &ft::Model::getState() { return _objectState; }

ft::Model &ft::Model::scale(const glm::vec3 &v, bool global) {
  (void)global;
  auto m = glm::scale(glm::mat4(1.0f), v);
  _objectState.scaling *= m;
  _node->state.modelMatrix =
      _objectState.translation * _objectState.rotation * _objectState.scaling;
  _centroid = m * glm::vec4(_oldCentroid, 1.0f);
  _node->state.updated = true;
  for (auto &n : _allNodes)
    n.second->state.updated = true;
  return *this;
}

ft::Model &ft::Model::rotate(const glm::vec3 &v, float a, bool global) {

  if (global) {
    auto m = glm::rotate(glm::mat4(1.0f), glm::radians(a), v);
    _objectState.rotation = _objectState.rotation * m;
    _node->state.modelMatrix =
        _objectState.translation * _objectState.rotation * _objectState.scaling;
  } else {
    auto m = glm::rotate(glm::mat4(1.0f), glm::radians(a), v);
    _objectState.rotation = _objectState.rotation * m;
    _node->state.modelMatrix =
        _objectState.translation * _objectState.rotation * _objectState.scaling;
  }
  _centroid = _node->state.modelMatrix * glm::vec4(_oldCentroid, 1.0f);
  _node->state.updated = true;
  for (auto &n : _allNodes)
    n.second->state.updated = true;
  return *this;
}

ft::Model &ft::Model::translate(const glm::vec3 &v, bool global) {

  if (global) {
    auto m = glm::translate(glm::mat4(1.0f), v);
    _objectState.translation = m * _objectState.translation;

  } else {
    glm::vec3 t = _objectState.rotation * glm::vec4(v, 0.0f);
    auto m = glm::translate(glm::mat4(1.0f), t);
    _objectState.translation = m * _objectState.translation;
  }

  _node->state.modelMatrix =
      _objectState.translation * _objectState.rotation * _objectState.scaling;
  _centroid = _node->state.modelMatrix * glm::vec4(_oldCentroid, 1.0f);
  _node->state.updated = true;
  for (auto &n : _allNodes)
    n.second->state.updated = true;
  return *this;
}
ft::Model &ft::Model::scale(const glm::mat4 &scale) {

  _objectState.scaling = scale;
  _node->state.modelMatrix =
      _objectState.translation * _objectState.rotation * _objectState.scaling;
  _centroid = _node->state.modelMatrix * glm::vec4(_oldCentroid, 1.0f);
  _node->state.updated = true;
  for (auto &n : _allNodes)
    n.second->state.updated = true;

  return *this;
}

ft::Model &ft::Model::rotate(const glm::mat4 &rotation) {

  _objectState.rotation = rotation;
  _node->state.modelMatrix =
      _objectState.translation * _objectState.rotation * _objectState.scaling;
  _centroid = _node->state.modelMatrix * glm::vec4(_oldCentroid, 1.0f);
  _node->state.updated = true;
  for (auto &n : _allNodes)
    n.second->state.updated = true;

  return *this;
}

ft::Model &ft::Model::translate(const glm::mat4 &translate) {

  _objectState.translation = translate;
  _node->state.modelMatrix =
      _objectState.translation * _objectState.rotation * _objectState.scaling;
  _centroid = _node->state.modelMatrix * glm::vec4(_oldCentroid, 1.0f);
  _node->state.updated = true;
  for (auto &n : _allNodes)
    n.second->state.updated = true;

  return *this;
}

// private
void ft::Model::drawNode(const CommandBuffer::pointer &commandBuffer,
                         Node *node, const GraphicsPipeline::pointer &pipeline,
                         const std::function<void(const Primitive &)> &fun,
                         const VkShaderStageFlags stages) {
  if (node->state.flags & ft::MODEL_HIDDEN_BIT)
    return;

  if (node->state.updated) {
    node->state.updatedMatrix = node->state.modelMatrix;
    Node *currentParent = node->parent;
    while (currentParent) {
      node->state.updatedMatrix =
          currentParent->state.modelMatrix * node->state.updatedMatrix;
      currentParent = currentParent->parent;
    }
    node->state.updated = false;
  }

  // push constants
  PushConstantObject push{node->state.updatedMatrix, node->state.baseColor,
                          node->state.id};

  vkCmdPushConstants(commandBuffer->getVKCommandBuffer(),
                     pipeline->getVKPipelineLayout(), stages, 0,
                     sizeof(PushConstantObject), &push);

  //   if (hasGeom)
  //     vkCmdPushConstants(commandBuffer->getVKCommandBuffer(),
  //                        pipeline->getVKPipelineLayout(),
  //                        VK_SHADER_STAGE_ALL, sizeof(PushConstantObject),
  //                        sizeof(glm::mat4),
  //                        &(node->state.updatedMatrix));
  //
  for (const auto &p : node->mesh) {
    if (p.indexCount > 0) {
      fun(p);
      vkCmdDrawIndexed(commandBuffer->getVKCommandBuffer(), p.indexCount, 1,
                       p.firstIndex, 0, 0);
    }
  }
  for (auto &n : node->children)
    drawNode(commandBuffer, n, pipeline, fun, stages);
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

glm::vec3 ft::Model::getCentroid() const { return _centroid; }

std::pair<glm::vec3, glm::vec3> ft::Model::getAABB() const {
  return {_aabb.min, _aabb.max};
}

/****************************************Gizmo*************************************/

ft::Gizmo::Gizmo(ft::Device::pointer device, const std::string &filePath,
                 uint32_t bufferCount)
    : _ftDevice(std::move(device)), _objectState{}, _matrix(glm::mat4(1.0f)) {

  tinygltf::Model gltfInput;
  tinygltf::TinyGLTF gltfContext;
  std::string error, warning;

  if (!gltfContext.LoadASCIIFromFile(&gltfInput, &error, &warning, filePath))
    throw std::runtime_error("Could not open GLTF file!\n" + error + "\n" +
                             warning);

  // load gltf scene
  tinygltf::Scene &scene = gltfInput.scenes[0];
  std::vector<Model::pointer> models;
  for (int i : scene.nodes) {
    const tinygltf::Node node = gltfInput.nodes[i];
    auto model = std::make_shared<Model>(_ftDevice, gltfInput, node,
                                         bufferCount, ft::LOAD_OPTION_NO_AABB);
    if (model->empty())
      continue;
    models.push_back(model);
  }

  assert(models.size() > 6);

  // combine the meshes
  uint32_t vOffset = 0;
  uint32_t iOffset = 0;

  for (auto &model : models) {
    vOffset += model->getVertices().size();
    iOffset += model->getIndices().size();
  }

  _vertices.reserve(vOffset);
  _indices.reserve(iOffset);
  vOffset = 0;
  iOffset = 0;
  for (auto &model : models) {
    // append the vertices
    _vertices.insert(_vertices.end(), model->getVertices().begin(),
                     model->getVertices().end());
    // append the indices
    auto i = _indices.insert(_indices.end(), model->getIndices().begin(),
                             model->getIndices().end());
    std::for_each(i, _indices.end(), [&vOffset](uint32_t &e) { e += vOffset; });
    // add a primitive to the mesh
    Primitive p = {};
    p.firstIndex = iOffset;
    p.indexCount = model->getIndices().size();
    p.id = model->getID();
    p.flags = ft::MODEL_SIMPLE_BIT;
    _mesh.push_back(p);
    // calculate the offsets
    vOffset += model->getVertices().size();
    iOffset += model->getIndices().size();
  }

  _mesh[0].color = {0.99f, 0.1f, 0.45f};
  _mesh[1].color = _mesh[4].color = {0.0f, 0.0f, 0.99f};
  _mesh[2].color = _mesh[5].color = {0.0f, 0.99f, 0.0f};
  _mesh[3].color = _mesh[6].color = {0.99f, 0.0f, 0.0f};
  _mesh[1].color2 = _mesh[4].color2 = {0.27f, 0.27f, 0.99f};
  _mesh[2].color2 = _mesh[5].color2 = {0.31f, 0.99f, 0.31f};
  _mesh[3].color2 = _mesh[6].color2 = {0.99f, 0.27f, 0.27f};

  _mesh[0].element = Elements::NIL;
  _mesh[1].element = Elements::Z_ARROW;
  _mesh[2].element = Elements::Y_ARROW;
  _mesh[3].element = Elements::X_ARROW;
  _mesh[4].element = Elements::Z_RING;
  _mesh[5].element = Elements::Y_RING;
  _mesh[6].element = Elements::X_RING;

  createVertexBuffer();
  createIndexBuffer();
};

void ft::Gizmo::bind(const CommandBuffer::pointer &commandBuffer,
                     uint32_t index) {
  (void)index;
  VkBuffer vertexBuffers[] = {_ftVertexBuffer->getVKBuffer()};
  VkDeviceSize offsets[] = {0};
  vkCmdBindVertexBuffers(commandBuffer->getVKCommandBuffer(), 0, 1,
                         vertexBuffers, offsets);
  vkCmdBindIndexBuffer(commandBuffer->getVKCommandBuffer(),
                       _ftIndexBuffer->getVKBuffer(), 0, VK_INDEX_TYPE_UINT32);
}

void ft::Gizmo::draw(const CommandBuffer::pointer &commandBuffer,
                     const GraphicsPipeline::pointer &pipeline,
                     const VkShaderStageFlags stages) {
  // push constants
  for (const auto &p : _mesh) {
    if (p.flags & ft::MODEL_HIDDEN_BIT)
      continue;
    if (p.indexCount > 0) {
      PushConstantObject push{_matrix, p.id == _selectedId ? p.color2 : p.color,
                              p.id};
      vkCmdPushConstants(commandBuffer->getVKCommandBuffer(),
                         pipeline->getVKPipelineLayout(), stages, 0,
                         sizeof(PushConstantObject), &push);

      vkCmdDrawIndexed(commandBuffer->getVKCommandBuffer(), p.indexCount, 1,
                       p.firstIndex, 0, 0);
    }
  }
}

ft::Gizmo &ft::Gizmo::aabb(const glm::vec3 &min, const glm::vec3 &max) {
  glm::vec3 center = 0.5f * (max + min);
  glm::vec3 scale = 0.2f * (max - min);
  return at(center).scale(scale);
}

ft::Gizmo &ft::Gizmo::at(const glm::vec3 &transitionV) {
  _objectState.translation =
      glm::translate(glm::mat4(1.0f), transitionV) * _objectState.translation;
  _matrix =
      _objectState.translation * _objectState.rotation * _objectState.scaling;
  return *this;
}

ft::Gizmo &ft::Gizmo::rotate(const glm::vec3 &rotationV, float alpha) {
  _objectState.rotation =
      glm::rotate(glm::mat4(1.0f), glm::radians(alpha), rotationV) *
      _objectState.rotation;
  _matrix =
      _objectState.translation * _objectState.rotation * _objectState.scaling;
  return *this;
}
ft::Gizmo &ft::Gizmo::scale(const glm::vec3 &scaleV) {
  _objectState.scaling =
      glm::scale(glm::mat4(1.0f), scaleV) * _objectState.scaling;
  _matrix =
      _objectState.translation * _objectState.rotation * _objectState.scaling;
  return *this;
}

ft::Gizmo &ft::Gizmo::resetTransform() {
  _objectState.translation = glm::mat4(1.0f);
  _objectState.rotation = glm::mat4(1.0f);
  _objectState.scaling = glm::mat4(1.0f);
  _matrix = glm::mat4(1.0f);
  return *this;
}

ft::Gizmo &ft::Gizmo::setScale(const glm::mat4 &scale) {
  _objectState.scaling = scale;
  _matrix =
      _objectState.translation * _objectState.rotation * _objectState.scaling;
  return *this;
}

ft::Gizmo &ft::Gizmo::setRotation(const glm::mat4 &rot) {
  _objectState.rotation = rot;
  _matrix =
      _objectState.translation * _objectState.rotation * _objectState.scaling;
  return *this;
}

ft::Gizmo &ft::Gizmo::setTranslation(const glm::mat4 &trs) {
  _objectState.translation = trs;
  _matrix =
      _objectState.translation * _objectState.rotation * _objectState.scaling;
  return *this;
}

ft::Gizmo &ft::Gizmo::setTransform(ft::ObjectState state) {
  _objectState = state;
  _matrix =
      _objectState.translation * _objectState.rotation * _objectState.scaling;
  return *this;
}

void ft::Gizmo::createVertexBuffer() {
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

void ft::Gizmo::createIndexBuffer() {
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

void ft::Gizmo::printInfo() const {
  std::cout << "Gizmo:" << std::endl;
  std::cout << "\tvertices: " << _vertices.size() << "\n";
  std::cout << "\tindices: " << _indices.size() << "\n";
}

ft::Gizmo::Elements ft::Gizmo::getSelected() const {

  if (_selectedId)
    for (auto &p : _mesh) {
      if (p.id == _selectedId)
        return p.element;
    }

  return Elements::NIL;
}

bool ft::Gizmo::select(uint32_t id) {
  for (auto &p : _mesh) {
    if (p.id == id) {
      _selectedId = id;
      return true;
    }
  }
  return false;
}

void ft::Gizmo::unselect() { _selectedId = 0u; }

bool ft::Gizmo::isSelected() const { return _selectedId != 0u; }

/****************************************BoundingBox*************************************/

ft::BoundingBox::BoundingBox(ft::Device::pointer device,
                             const std::string &filePath, uint32_t bufferCount)
    : _ftDevice(std::move(device)), _objectState{}, _matrix(glm::mat4(1.0f)) {
  //
  //  tinygltf::Model gltfInput;
  //  tinygltf::TinyGLTF gltfContext;
  //  std::string error, warning;
  //
  //  if (!gltfContext.LoadASCIIFromFile(&gltfInput, &error, &warning,
  //  filePath))
  //    throw std::runtime_error("Could not open GLTF file!\n" + error + "\n" +
  //                             warning);
  //
  //  // load gltf scene
  //  tinygltf::Scene &scene = gltfInput.scenes[0];
  //  std::vector<Model::pointer> models;
  //  for (int i : scene.nodes) {
  //    const tinygltf::Node node = gltfInput.nodes[i];
  //    auto model =
  //        std::make_shared<Model>(_ftDevice, gltfInput, node, bufferCount);
  //    if (model->empty())
  //      continue;
  //    models.push_back(model);
  //  }
  //
  //  assert(models.size() > 0);
  //
  //  _vertices = std::move(models[0]->getVertices());
  //  _indices = std::move(models[0]->getIndices());
  //  _id = models[0]->getID();
  (void)filePath;
  (void)bufferCount;
  _vertices.reserve(8);
  std::vector<glm::vec3> vertices = {
      {-1.0f, -1.0f, -1.0f}, // 0
      {1.0f, -1.0f, -1.0f},  // 1
      {1.0f, 1.0f, -1.0f},   // 2
      {-1.0f, 1.0f, -1.0f},  // 3
      {-1.0f, -1.0f, 1.0f},  // 4
      {1.0f, -1.0f, 1.0f},   // 5
      {1.0f, 1.0f, 1.0f},    // 6
      {-1.0f, 1.0f, 1.0f}    // 7
  };

  for (auto p : vertices) {
    Vertex v = {};
    v.pos = glm::normalize(p);
    _vertices.push_back(v);
  }

  _indices = {
      // Bottom face
      0, 1, 1, 5, 5, 4, 4, 0, 0, 5, 1, 4, // Diagonals of the bottom face
      // Top face
      3, 2, 2, 6, 6, 7, 7, 3, 3, 6, 2, 7, // Diagonals of the top face
      // Right face
      1, 2, 2, 6, 6, 5, 5, 1, 1, 6, 2, 5, // Diagonals of the right face
      // Left face
      0, 3, 3, 7, 7, 4, 4, 0, 0, 7, 3, 4, // Diagonals of the left face
      // Front face
      0, 1, 1, 2, 2, 3, 3, 0, 0, 2, 1, 3, // Diagonals of the front face
      // Back face
      4, 5, 5, 6, 6, 7, 7, 4, 4, 6, 5, 7 // Diagonals of the back face
  };
  _color = {0.0f, 1.0f, 0.1f};
  _matrix = glm::mat4(1.0f);

  createVertexBuffer();
  createIndexBuffer();
};

void ft::BoundingBox::bind(const CommandBuffer::pointer &commandBuffer,
                           uint32_t index) {
  (void)index;
  VkBuffer vertexBuffers[] = {_ftVertexBuffer->getVKBuffer()};
  VkDeviceSize offsets[] = {0};
  vkCmdBindVertexBuffers(commandBuffer->getVKCommandBuffer(), 0, 1,
                         vertexBuffers, offsets);
  vkCmdBindIndexBuffer(commandBuffer->getVKCommandBuffer(),
                       _ftIndexBuffer->getVKBuffer(), 0, VK_INDEX_TYPE_UINT32);
}

void ft::BoundingBox::draw(const CommandBuffer::pointer &commandBuffer,
                           const GraphicsPipeline::pointer &pipeline,
                           const VkShaderStageFlags stages) {

  // push constants
  PushConstantObject push{_matrix, _color, _id};
  vkCmdPushConstants(commandBuffer->getVKCommandBuffer(),
                     pipeline->getVKPipelineLayout(), stages, 0,
                     sizeof(PushConstantObject), &push);

  vkCmdDrawIndexed(commandBuffer->getVKCommandBuffer(),
                   static_cast<uint32_t>(_indices.size()), 1, 0, 0, 0);
}

ft::BoundingBox &ft::BoundingBox::aabb(const glm::vec3 &min,
                                       const glm::vec3 &max) {
  glm::vec3 center = 0.5f * (max + min);
  glm::vec3 scale = (max - min);
  (void)center;
  return this->scale(scale);
  // return at(center).scale(scale);
}

ft::BoundingBox &ft::BoundingBox::at(const glm::vec3 &transitionV) {
  _objectState.translation *= glm::translate(glm::mat4(1.0f), transitionV);
  _matrix =
      _objectState.translation * _objectState.rotation * _objectState.scaling;
  return *this;
}

ft::BoundingBox &ft::BoundingBox::rotate(const glm::vec3 &rotationV,
                                         float alpha) {
  _objectState.rotation *=
      glm::rotate(glm::mat4(1.0f), glm::radians(alpha), rotationV);
  _matrix =
      _objectState.translation * _objectState.rotation * _objectState.scaling;
  return *this;
}
ft::BoundingBox &ft::BoundingBox::scale(const glm::vec3 &scaleV) {
  _objectState.scaling *= glm::scale(glm::mat4(1.0f), scaleV);
  _matrix =
      _objectState.translation * _objectState.rotation * _objectState.scaling;
  return *this;
}

ft::BoundingBox &ft::BoundingBox::setScale(const glm::mat4 &scale) {
  _objectState.scaling = scale;
  _matrix =
      _objectState.translation * _objectState.rotation * _objectState.scaling;
  return *this;
}

ft::BoundingBox &ft::BoundingBox::setRotation(const glm::mat4 &rot) {
  _objectState.rotation = rot;
  _matrix =
      _objectState.translation * _objectState.rotation * _objectState.scaling;
  return *this;
}

ft::BoundingBox &ft::BoundingBox::setTranslation(const glm::mat4 &trs) {
  _objectState.translation = trs;
  _matrix =
      _objectState.translation * _objectState.rotation * _objectState.scaling;
  return *this;
}

ft::BoundingBox &ft::BoundingBox::resetTransform() {
  _objectState.translation = glm::mat4(1.0f);
  _objectState.rotation = glm::mat4(1.0f);
  _objectState.scaling = glm::mat4(1.0f);
  _matrix = glm::mat4(1.0f);
  return *this;
}
ft::BoundingBox &ft::BoundingBox::setTransform(ft::ObjectState state) {
  _objectState = state;
  _matrix =
      _objectState.translation * _objectState.rotation * _objectState.scaling;
  return *this;
}

void ft::BoundingBox::createVertexBuffer() {
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

void ft::BoundingBox::createIndexBuffer() {
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

void ft::BoundingBox::printInfo() const {
  std::cout << "BoundingBox: " << _id << "\n";
  std::cout << "\tvertices: " << _vertices.size() << "\n";
  std::cout << "\tindices: " << _indices.size() << "\n";
  for (auto &v : _vertices)
    std::cout << "\t\t" << glm::to_string(v.pos) << "\n";
  for (auto &v : _indices)
    std::cout << v << " ";
  std::cout << "\n";
}

std::string ft::Model::getPath() const { return _modelPath; }
void ft::Model::setPath(const std::string &path) { _modelPath = path; }
