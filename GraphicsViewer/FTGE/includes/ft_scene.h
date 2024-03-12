#ifndef FTGRAPHICS_FT_SCENE_H
#define FTGRAPHICS_FT_SCENE_H

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
#include "ft_swapChain.h"
#include "ft_texture.h"
#include "ft_tools.h"
#include "ft_vertex.h"
#include <cstdint>

namespace ft {

class Scene {

public:
  using pointer = std::shared_ptr<Scene>;

  Scene(Device::pointer device, std::vector<Buffer::pointer> ubos);
  ~Scene() = default;

  // drawing
  void drawInstancedObjs(const CommandBuffer::pointer &commandBuffer,
                         const GraphicsPipeline::pointer &pipeline,
                         uint32_t index);
  void drawSimpleObjs(const CommandBuffer::pointer &,
                      const GraphicsPipeline::pointer &,
                      const SimpleRdrSys::pointer &, uint32_t index);
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

  // add objects to the scene
  Model::pointer addModelFromObj(const std::string &objectPath,
                                 ft::InstanceData data);
  std::vector<Model::pointer>
  addDoubleTexturedFromGltf(const std::string &,
                            const DescriptorPool::pointer &,
                            const DescriptorSetLayout::pointer &);
  std::vector<Model::pointer>
  addSingleTexturedFromGltf(const std::string &,
                            const DescriptorPool::pointer &,
                            const DescriptorSetLayout::pointer &);
  Model::pointer addModelFromGltf(const std::string &,
                                  const ft::InstanceData data);
  uint32_t addObjectCopyToTheScene(uint32_t id, InstanceData data);

  Model::pointer addCubeBox(const std::string &gltfModel,
                            const std::string &ktxTexture,
                            const DescriptorPool::pointer &pool,
                            const DescriptorSetLayout::pointer &layout,
                            const ft::InstanceData data);

  // set properties of the scene
  void addMaterialToObj(uint32_t id, Material::pointer texture);
  void addPointLightToTheScene(PointLightObject &pl);
  [[nodiscard]] Camera::pointer getCamera() const;
  void setCamera(Camera::pointer camera);
  void setGeneralLight(glm::vec3 color, glm::vec3 direction, float ambient);
  void updateCameraUBO();
  PointLightObject *getLights();
  [[nodiscard]] std::vector<Model::pointer> getModels() const;
  void setMaterialPool(TexturePool::pointer pool);
  bool select(uint32_t id);
  void unselectAll();
  void createCubeMapTexture(const std::string &path);

private:
  Device::pointer _ftDevice;
  std::vector<Model::pointer> _models;
  std::vector<Buffer::pointer> _ftUniformBuffers;
  Camera::pointer _camera;
  UniformBufferObject _ubo;
  TexturePool::pointer _ftTexturePool;
  std::map<uint32_t, std::vector<Model::pointer>> _materialToModel;
  Texture::pointer _ftCubeTexture;
};

} // namespace ft

#endif // FTGRAPHICS_FT_SCENE_H
