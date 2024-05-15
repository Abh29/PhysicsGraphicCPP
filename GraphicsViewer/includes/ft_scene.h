#ifndef FTGRAPHICS_FT_SCENE_H
#define FTGRAPHICS_FT_SCENE_H

#include "ft_body.h"
#include "ft_buffer.h"
#include "ft_camera.h"
#include "ft_command.h"
#include "ft_defines.h"
#include "ft_descriptor.h"
#include "ft_device.h"
#include "ft_headers.h"
#include "ft_model.h"
#include "ft_pipeline.h"
#include "ft_rendering_systems.h"
#include "ft_sceneObject.h"
#include "ft_swapChain.h"
#include "ft_texture.h"
#include "ft_threads.h"
#include "ft_tools.h"
#include "ft_vertex.h"
#include <cstdint>
#include <memory>

namespace ft {

class Scene {

public:
  using pointer = std::shared_ptr<Scene>;

  // this is used to keep truck of the input files for saving
  enum class SceneNodeType {
    OBJ_SIMPLE,
    GLTF_SIMPLE,
    GLTF_SINGLE_TEX,
    GLTF_DOUBLE_TEX,
    SKY_BOX,
  };

  struct SceneNode {
    std::string _inputFile;
    std::vector<Model::pointer> _models;
    SceneNodeType _type;
    std::string _texturePath;
  };

  Scene(Device::pointer device, std::vector<Buffer::pointer> ubos);
  ~Scene() = default;

  // drawing
  void drawInstancedObjs(const CommandBuffer::pointer &commandBuffer,
                         const GraphicsPipeline::pointer &pipeline,
                         uint32_t index);

  void drawSimpleObjs(const CommandBuffer::pointer &,
                      const GraphicsPipeline::pointer &,
                      const SimpleRdrSys::pointer &, uint32_t index);

  void drawSimpleObjsWithOutline(const CommandBuffer::pointer &,
                                 const SimpleRdrSys::pointer &,
                                 const OutlineRdrSys::pointer &, uint32_t);

  void drawTexturedObjs(const CommandBuffer::pointer &,
                        const GraphicsPipeline::pointer &,
                        const OneTextureRdrSys::pointer &, uint32_t index);

  void draw2TexturedObjs(const CommandBuffer::pointer &,
                         const GraphicsPipeline::pointer &,
                         const TwoTextureRdrSys::pointer &, uint32_t index);

  void drawSkyBox(const CommandBuffer::pointer &,
                  const GraphicsPipeline::pointer &,
                  const SkyBoxRdrSys::pointer &, uint32_t index);

  void drawPickObjs(const CommandBuffer::pointer &,
                    const GraphicsPipeline::pointer &, uint32_t index);

  void drawOulines(const CommandBuffer::pointer &,
                   const SimpleRdrSys::pointer &,
                   const OutlineRdrSys::pointer &, uint32_t index);

  void drawPointsTopology(const CommandBuffer::pointer &,
                          const SimpleRdrSys::pointer &,
                          const PointRdrSys::pointer &, uint32_t index);

  void drawLinesTopology(const CommandBuffer::pointer &,
                         const SimpleRdrSys::pointer &,
                         const LineRdrSys::pointer &, uint32_t index);

  void drawNormals(const CommandBuffer::pointer &,
                   const SimpleRdrSys::pointer &,
                   const NormDebugRdrSys::pointer &, uint32_t index);

  // add objects to the scene
  SceneObject::pointer addModelFromObj(const std::string &objectPath,
                                       const ft::ObjectState &data);

  std::vector<SceneObject::pointer> addDoubleTexturedFromGltf(
      const std::string &, const DescriptorPool::pointer &,
      const DescriptorSetLayout::pointer &, const ft::ObjectState &);

  std::vector<SceneObject::pointer> addSingleTexturedFromGltf(
      const std::string &, const DescriptorPool::pointer &,
      const DescriptorSetLayout::pointer &, const ft::ObjectState &);

  std::vector<SceneObject::pointer>
  addModelFromGltf(const std::string &, const ft::ObjectState &data);

  uint32_t addObjectCopyToTheScene(uint32_t id, InstanceData data);

  SceneObject::pointer addCubeBox(const std::string &gltfModel,
                                  const std::string &ktxTexture,
                                  const DescriptorPool::pointer &pool,
                                  const DescriptorSetLayout::pointer &layout,
                                  const ft::ObjectState data);

  // camera
  void addCamera(Camera::pointer camera);
  [[nodiscard]] Camera::pointer getCamera() const;
  void removeCurrentCamera();
  void nextCamera();
  std::vector<Camera::pointer> &getAllCameras();
  bool hasCamera() const;

  ft::Gizmo::pointer loadGizmo(const std::string &gltfModel);
  ft::Gizmo::pointer getGizmo() const;
  bool hasGizmo() const;

  // scene update
  void updateSceneObjects(float duration, ft::ThreadPool::pointer &pool);
  void updateCameraUBO();

  // set properties of the scene
  void addMaterialToObj(uint32_t id, Material::pointer texture);
  void addPointLightToTheScene(PointLightObject &pl);
  void setGeneralLight(glm::vec3 color, glm::vec3 direction, float ambient);
  PointLightObject *getLights();
  [[nodiscard]] std::vector<SceneObject::pointer> &getObjects();
  void setMaterialPool(TexturePool::pointer pool);
  bool select(uint32_t id);
  void unselectAll();
  void createCubeMapTexture(const std::string &path);
  void hideSelected();
  void unhideSelected();
  void unhideAll();
  void resetAll();
  void togglePointsTopo();
  void toggleLinesTopo();
  void toggleNormalDebug();
  void showSelectedInfo() const;
  ft::Model::raw_ptr getSelectedModel() const;
  void toggleGizmo();
  bool isGlobalGizmo() const;
  bool hasSkyBox() const;
  UniformBufferObject &getUBO();

  // testing features
  std::vector<SceneNode> &getSceneGraph();
  void calculateNormals();

private:
  struct State {
    Model::raw_ptr lastSelect = nullptr;
    bool globalGizbo = false;
    bool hasSkyBox = false;
  };

  Device::pointer _ftDevice;
  std::vector<SceneObject::pointer> _objects;
  std::vector<Buffer::pointer> _ftUniformBuffers;
  std::vector<Camera::pointer> _cameras;
  uint32_t _currentCamera = -1u;
  UniformBufferObject _ubo;
  TexturePool::pointer _ftTexturePool;
  std::map<uint32_t, std::vector<Model::pointer>> _materialToModel;
  Texture::pointer _ftCubeTexture;
  Gizmo::pointer _ftGizmo;
  State _state;
  std::vector<SceneNode> _sceneGraph;
};

} // namespace ft

#endif // FTGRAPHICS_FT_SCENE_H
