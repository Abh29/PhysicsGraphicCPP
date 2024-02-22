#ifndef FTGRAPHICS_FT_TEXTURE_H
#define FTGRAPHICS_FT_TEXTURE_H

#include "ft_buffer.h"
#include "ft_defines.h"
#include "ft_descriptor.h"
#include "ft_device.h"
#include "ft_headers.h"
#include "ft_image.h"
#include "ft_sampler.h"

namespace ft {

class Texture {

public:
  using pointer = std::shared_ptr<Texture>;

  enum class FileType { FT_TEXTURE_KTX, FT_TEXTURE_PNG, FT_TEXTURE_UNDEFINED };

  Texture(Device::pointer, std::string texture,
          FileType type = FileType::FT_TEXTURE_UNDEFINED);
  ~Texture() = default;

  [[nodiscard]] Image::pointer getTextureImage();
  [[nodiscard]] uint32_t getID() const;
  [[nodiscard]] std::string getTexturePath() const;
  [[nodiscard]] Sampler::pointer getSampler() const;

  [[nodiscard]] DescriptorSet::pointer getDescriptorSet(uint32_t index) const;
  void createDescriptorSets(DescriptorSetLayout::pointer layout,
                            DescriptorPool::pointer pool);
  void setDescriptorSets(std::vector<DescriptorSet::pointer> descriptorSets);
  std::vector<DescriptorSet::pointer> &getDescriptorSets();

  bool operator==(const Texture &m) const;

private:
  void createTextureResource();
  void createTextureFromKTXFile();
  void createTextureFromPNGFile();
  static uint32_t ID();

  uint32_t _id;
  Device::pointer _ftDevice;
  Sampler::pointer _ftSampler;
  std::string _path;
  Image::pointer _ftTextureImage;
  std::vector<DescriptorSet::pointer> _ftDescriptorSets;
};

class Material {

public:
  using pointer = std::shared_ptr<Material>;

  Material(Device::pointer device);
  ~Material() = default;

  ft::DescriptorSet::pointer getDescriptorSet(uint32_t index);
  void addTexture(Texture::pointer texture);
  void createDescriptors(DescriptorPool::pointer pool,
                         DescriptorSetLayout::pointer layout);
  void bindDescriptor(uint32_t frameIndex, uint32_t textureIndex,
                      uint32_t binding);

  void setColorFactor(glm::vec4 v);
  void setAlphaMode(std::string a);
  void setAlphaCutOff(float a);
  void setDoubleSided(bool v);

  glm::vec4 getColorFactor() const;
  std::string getAlphaMode() const;
  float getAlphaCutOff() const;
  bool isDoubleSided() const;
  uint32_t getTexturesSize() const;
  Texture::pointer getTexture(uint32_t index) const;

private:
  Device::pointer _ftDevice;
  std::vector<DescriptorSet::pointer> _ftDescriptorSets;
  std::vector<Texture::pointer> _ftTextures;
  glm::vec4 _colorFactor = glm::vec4(1.0f);
  std::string _alphaMode = "OPAQUE";
  float _alphaCutOff = 0;
  bool _doubleSided = false;
};

class TexturePool {

public:
  using pointer = std::shared_ptr<TexturePool>;
  explicit TexturePool(Device::pointer device);
  ~TexturePool() = default;

  Texture::pointer createTexture(std::string path, Texture::FileType fileType);
  Texture::pointer getTextureByID(uint32_t id);
  uint32_t addMaterial(Material::pointer m);
  Material::pointer getMaterialByID(uint32_t id);

private:
  Device::pointer _ftDevice;
  std::map<std::string, Texture::pointer> _textures;
  std::vector<Material::pointer> _materials;
  std::map<uint32_t, Texture::pointer> _ids;
};

} // namespace ft

#endif // FTGRAPHICS_FT_TEXTURE_H
