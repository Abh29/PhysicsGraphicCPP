#ifndef FTGRAPHICS_FT_MODEL_H
#define FTGRAPHICS_FT_MODEL_H

#include "ft_buffer.h"
#include "ft_command.h"
#include "ft_defines.h"
#include "ft_device.h"
#include "ft_headers.h"
#include "ft_pipeline.h"
#include "ft_texture.h"
#include "ft_vertex.h"
#include <cstdint>
#include <glm/detail/qualifier.hpp>
#include <glm/fwd.hpp>
#include <utility>
#include <vector>
#include <vulkan/vulkan_core.h>

#define MAX_COPY_COUNT 100

namespace ft {

class Model {
public:
  struct ModelState {
    uint32_t id;
    uint32_t flags;
    uint32_t instanceCount;
    std::string name;
    glm::vec3 baseColor;
    glm::mat4 modelMatrix;
    glm::mat4 normalMatrix;
    glm::mat4 updatedMatrix = glm::mat4(1.0f);
    bool updated = true;
  };

  struct ModelInstData {
    std::vector<uint32_t> ids;
    std::vector<InstanceData> data;
    std::vector<uint32_t> flags;
  };

  struct Primitive {
    uint32_t firstIndex;
    uint32_t indexCount;
    uint32_t materialIndex;
    Material::pointer material;
  };

  struct Node {
    Node *parent;
    std::vector<Node *> children;
    std::vector<Primitive> mesh;
    ModelState state;
    ~Node() {
      for (auto &child : children)
        delete child;
    }
  };

  struct AABB {
    glm::vec3 min;
    glm::vec3 max;
    uint32_t firstIndex;
    uint32_t indexCount;
    uint32_t flags;
    glm::vec3 color;
  };

  using pointer = std::shared_ptr<Model>;
  using wpointer = std::weak_ptr<Model>;
  using raw_ptr = Model *;

  Model(Device::pointer device, std::string filePath, uint32_t bufferCount,
        uint32_t options = 0);

  Model(Device::pointer device, const tinygltf::Model &gltfInput,
        const tinygltf::Node &node, uint32_t bufferCount, uint32_t options = 0);

  ~Model();

  static uint32_t ID();
  [[nodiscard]] Model::pointer clone() const;
  uint32_t addCopy(const InstanceData &copyData = {});

  void bind(const CommandBuffer::pointer &commandBuffer, uint32_t index);
  void draw(const CommandBuffer::pointer &commandBuffer,
            const GraphicsPipeline::pointer &pipeline,
            const VkShaderStageFlags stages = VK_SHADER_STAGE_VERTEX_BIT);
  void drawAABB(const CommandBuffer::pointer &commandBuffer,
                const GraphicsPipeline::pointer &pipeline,
                const VkShaderStageFlags stages = VK_SHADER_STAGE_VERTEX_BIT);
  void
  draw_extended(const CommandBuffer::pointer &,
                const GraphicsPipeline::pointer &,
                const std::function<void(const Primitive &)> &fun,
                const VkShaderStageFlags stages = VK_SHADER_STAGE_VERTEX_BIT);

  void updateVertexBuffer();
  void reshade();
  [[nodiscard]] bool findID(uint32_t id) const;
  [[nodiscard]] uint32_t getID() const;
  [[nodiscard]] std::vector<Node *> getAllNodes() const;
  glm::mat4 getModelMatrix() const;
  void addMaterial(const Material::pointer &material);
  glm::mat4 &getRootModelMatrix();

  // flag manager
  [[nodiscard]] bool isSelected() const;
  [[nodiscard]] bool empty() const;
  bool select(uint32_t id);
  bool unselect(uint32_t id);
  void selectAll();
  void unselectAll();
  bool overrideFlags(uint32_t id, uint32_t flags);
  bool setFlags(uint32_t id, uint32_t flags);
  bool unsetFlags(uint32_t id, uint32_t flags);
  bool toggleFlags(uint32_t id, uint32_t flags);
  [[nodiscard]] bool hasFlag(uint32_t flag) const;
  [[nodiscard]] bool hasNodeFlag(uint32_t id, uint32_t flag) const;
  [[nodiscard]] bool hasMaterial();
  [[nodiscard]] bool isUpdated() const;
  inline std::vector<Vertex> &getVertices() { return _vertices; };
  inline std::vector<uint32_t> &getIndices() { return _indices; };
  glm::vec3 getCentroid() const;
  std::pair<glm::vec3, glm::vec3> getAABB() const;

  // state manager
  void setState(const ft::ObjectState &data);
  ft::ObjectState getState() const;
  ft::ObjectState &getState();
  Model &scale(const glm::vec3 &v, bool global = false);
  Model &rotate(const glm::vec3 &v, float a, bool global = false);
  Model &translate(const glm::vec3 &v, bool global = false);

  // getters
  std::string getPath() const;
  void setPath(const std::string &path);

protected:
  Model() = default;
  void loadModel(uint32_t options = 0);
  void createAABB();
  void writePerInstanceData(uint32_t index);
  void createVertexBuffer();
  void createIndexBuffer();
  void loadNode(const tinygltf::Node &inputNode, const tinygltf::Model &input,
                Node *parent, uint32_t options = 0);
  void drawNode(const CommandBuffer::pointer &, Node *node,
                const GraphicsPipeline::pointer &,
                const std::function<void(const Primitive &)> &,
                const VkShaderStageFlags);

  static void ignored(const Primitive &primitive);

  Device::pointer _ftDevice;
  ModelInstData _instances;
  Buffer::pointer _ftVertexBuffer;
  std::vector<Buffer::pointer> _ftInstanceBuffers;
  Buffer::pointer _ftIndexBuffer;
  std::vector<Vertex> _vertices;
  std::vector<uint32_t> _indices;
  std::string _modelPath;
  Node *_node = nullptr;
  std::map<uint32_t, Node *> _allNodes;
  glm::vec3 _centroid;
  glm::vec3 _oldCentroid;
  ft::ObjectState _objectState;
  AABB _aabb = {};
};

class Gizmo {

public:
  using pointer = std::shared_ptr<Gizmo>;
  using raw_ptr = Gizmo *;

  enum class Elements {
    NIL,
    X_ARROW,
    Y_ARROW,
    Z_ARROW,
    X_RING,
    Y_RING,
    Z_RING,
  };

  Gizmo(Device::pointer device, const std::string &filePath,
        uint32_t bufferCount);
  ~Gizmo() = default;

  void bind(const CommandBuffer::pointer &commandBuffer, uint32_t index);
  void draw(const CommandBuffer::pointer &commandBuffer,
            const GraphicsPipeline::pointer &pipeline,
            const VkShaderStageFlags stages = VK_SHADER_STAGE_VERTEX_BIT);
  Gizmo &aabb(const glm::vec3 &min, const glm::vec3 &max);
  Gizmo &at(const glm::vec3 &transitionV);
  Gizmo &rotate(const glm::vec3 &rotationV, float alpha);
  Gizmo &scale(const glm::vec3 &scaleV);
  Gizmo &setScale(const glm::mat4 &scale);
  Gizmo &setRotation(const glm::mat4 &rotation);
  Gizmo &setTranslation(const glm::mat4 &tranlation);
  Gizmo &resetTransform();
  Gizmo &setTransform(ObjectState state);

  bool select(uint32_t id);
  Elements getSelected() const;
  void unselect();
  bool isSelected() const;
  void printInfo() const;

private:
  struct Primitive {
    uint32_t firstIndex;
    uint32_t indexCount;
    uint32_t id;
    uint32_t flags; // this could be eventuatlly used
    glm::vec3 color, color2;
    Elements element;
  };

  void createVertexBuffer();
  void createIndexBuffer();

  Device::pointer _ftDevice;
  Buffer::pointer _ftVertexBuffer;
  Buffer::pointer _ftIndexBuffer;
  std::vector<Vertex> _vertices;
  std::vector<uint32_t> _indices;
  std::vector<Primitive> _mesh;
  ft::ObjectState _objectState;
  glm::mat4 _matrix;
  uint32_t _selectedId, _minId, _maxId;
  bool _isSelected;
};

class BoundingBox {

public:
  using pointer = std::shared_ptr<BoundingBox>;

  BoundingBox(Device::pointer device, const std::string &filePath,
              uint32_t bufferCount);

  void bind(const CommandBuffer::pointer &commandBuffer, uint32_t index);
  void draw(const CommandBuffer::pointer &commandBuffer,
            const GraphicsPipeline::pointer &pipeline,
            const VkShaderStageFlags stages = VK_SHADER_STAGE_VERTEX_BIT);

  BoundingBox &aabb(const glm::vec3 &min, const glm::vec3 &max);
  BoundingBox &setScale(const glm::mat4 &scale);
  BoundingBox &setRotation(const glm::mat4 &rotation);
  BoundingBox &setTranslation(const glm::mat4 &tranlation);
  BoundingBox &at(const glm::vec3 &transitionV);
  BoundingBox &rotate(const glm::vec3 &rotationV, float alpha);
  BoundingBox &scale(const glm::vec3 &scaleV);
  BoundingBox &resetTransform();
  BoundingBox &setTransform(ObjectState state);

  void printInfo() const;

private:
  void createVertexBuffer();
  void createIndexBuffer();

  Device::pointer _ftDevice;
  Buffer::pointer _ftVertexBuffer;
  Buffer::pointer _ftIndexBuffer;
  std::vector<Vertex> _vertices;
  std::vector<uint32_t> _indices;
  ft::ObjectState _objectState;
  glm::mat4 _matrix;
  uint32_t _id;
  glm::vec3 _color;
};

} // namespace ft

#endif // FTGRAPHICS_FT_MODEL_H
