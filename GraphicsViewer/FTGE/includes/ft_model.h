#ifndef FTGRAPHICS_FT_MODEL_H
#define FTGRAPHICS_FT_MODEL_H

#include "ft_buffer.h"
#include "ft_command.h"
#include "ft_defines.h"
#include "ft_headers.h"
#include "ft_pipeline.h"
#include "ft_texture.h"
#include "ft_vertex.h"

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

  using pointer = std::shared_ptr<Model>;
  Model(Device::pointer device, std::string filePath, uint32_t bufferCount);

  Model(Device::pointer device, const tinygltf::Model &gltfInput,
        const tinygltf::Node &node, uint32_t bufferCount);

  ~Model();

  static uint32_t ID();
  [[nodiscard]] Model::pointer clone() const;
  uint32_t addCopy(const InstanceData &copyData = {});

  void bind(const CommandBuffer::pointer &commandBuffer, uint32_t index);
  void draw(const CommandBuffer::pointer &commandBuffer,
            const GraphicsPipeline::pointer &pipeline);
  void draw_extended(const CommandBuffer::pointer &,
                     const GraphicsPipeline::pointer &,
                     const std::function<void(const Primitive &)> &fun);

  [[nodiscard]] bool findID(uint32_t id) const;
  [[nodiscard]] uint32_t getID() const;
  [[nodiscard]] std::vector<Node *> getAllNodes() const;
  void setState(const InstanceData &data);
  void addMaterial(Material::pointer material);
  glm::mat4 &getRootModelMatrix();

  // state manager
  [[nodiscard]] bool isSelected() const;
  [[nodiscard]] bool empty() const;
  bool select(uint32_t id);
  bool unselect(uint32_t id);
  void selectAll();
  void unselectAll();
  bool overrideFlags(uint32_t id, uint32_t flags);
  bool setFlags(uint32_t id, uint32_t flags);
  bool unsetFlags(uint32_t id, uint32_t flags);
  [[nodiscard]] bool hasFlag(uint32_t flag) const;
  [[nodiscard]] bool hasNodeFlag(uint32_t id, uint32_t flag) const;
  [[nodiscard]] bool hasMaterial();
  [[nodiscard]] bool isUpdated() const;

private:
  Model() = default;
  void loadModel();
  void writePerInstanceData(uint32_t index);
  void createVertexBuffer();
  void createIndexBuffer();
  void loadNode(const tinygltf::Node &inputNode, const tinygltf::Model &input,
                Node *parent);
  void drawNode(const CommandBuffer::pointer &, Node *node,
                const GraphicsPipeline::pointer &,
                const std::function<void(const Primitive &)> &);

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
};

} // namespace ft

#endif // FTGRAPHICS_FT_MODEL_H
