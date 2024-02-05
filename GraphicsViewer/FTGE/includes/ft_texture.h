#ifndef FTGRAPHICS_FT_TEXTURE_H
#define FTGRAPHICS_FT_TEXTURE_H

#include "ft_headers.h"
#include "ft_defines.h"
#include "ft_device.h"
#include "ft_sampler.h"
#include "ft_image.h"
#include "ft_buffer.h"
#include "ft_descriptor.h"

namespace ft {

 class Texture {

 public:
     using pointer = std::shared_ptr<Texture>;

     enum class FileType {
         FT_TEXTURE_KTX,
         FT_TEXTURE_UNDEFINED
     };

    Texture(Device::pointer, std::string texture, FileType type = FileType::FT_TEXTURE_UNDEFINED);
    ~Texture() = default;

    [[nodiscard]] Image::pointer getTextureImage();
    [[nodiscard]] uint32_t getID() const;
    [[nodiscard]] std::string getTexturePath() const;
    [[nodiscard]] Sampler::pointer getSampler() const;

    [[nodiscard]] DescriptorSet::pointer getDescriptorSet(uint32_t index) const;
    void setDescriptorSets(std::vector<DescriptorSet::pointer> descriptorSets);
    std::vector<DescriptorSet::pointer>& getDescriptorSets();

    bool operator==(const Texture& m) const;

 private:
     void createTextureResource();
     void createTextureFromKTXFile();
     static uint32_t ID();

     uint32_t                                       _id;
     Device::pointer                                _ftDevice;
     Sampler::pointer                               _ftSampler;
     std::string                                    _path;
     Image::pointer                                 _ftTextureImage;
     std::vector<DescriptorSet::pointer>            _ftDescriptorSets;


 };

 class TexturePool {

 public:
     using pointer = std::shared_ptr<TexturePool>;
     explicit TexturePool(Device::pointer device);
     ~TexturePool() = default;

     Texture::pointer createTexture(std::string path, const DescriptorPool::pointer& pool, const DescriptorSetLayout::pointer& layout, Texture::FileType fileType);
     Texture::pointer getTextureByID(uint32_t id);
     uint32_t addMaterial(Material m);
     Material& getMaterialByID(uint32_t id);

 private:
     Device::pointer                                    _ftDevice;
     std::map<std::string, Texture::pointer>            _textures;
     std::vector<Material>                              _materials;
     std::map<uint32_t , Texture::pointer>              _ids;

 };

}

#endif //FTGRAPHICS_FT_TEXTURE_H
