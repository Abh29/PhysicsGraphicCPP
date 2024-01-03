#ifndef FTGRAPHICS_FT_MATERIAL_H
#define FTGRAPHICS_FT_MATERIAL_H

#include "ft_headers.h"
#include "ft_defines.h"
#include "ft_device.h"
#include "ft_sampler.h"
#include "ft_image.h"
#include "ft_buffer.h"

namespace ft {

 class Material {

 public:
     using pointer = std::shared_ptr<Material>;
    Material(Device::pointer, Sampler::pointer, std::string texture);
    ~Material() = default;

    [[nodiscard]] Image::pointer getTextureImage();
    [[nodiscard]] uint32_t getID() const;
    [[nodiscard]] std::string getTexturePath() const;
    [[nodiscard]] Sampler::pointer getSampler() const;

    bool operator==(const Material& m) const;

 private:
     void createTextureResource();
     static uint32_t ID();

     uint32_t                           _id;
     Device::pointer                    _ftDevice;
     Sampler::pointer                   _ftSampler;
     std::string                        _path;
     Image::pointer                     _ftTextureImage;

 };

 class MaterialPool {

 public:
     using pointer = std::shared_ptr<MaterialPool>;
     explicit MaterialPool(Device::pointer device);
     ~MaterialPool() = default;

     Material::pointer createMaterial(std::string path, Sampler::pointer sampler);
     Material::pointer getMaterialByID(uint32_t id);


 private:
     Device::pointer                                    _ftDevice;
     std::map<std::string, Material::pointer>           _textures;
     std::map<uint32_t , Material::pointer>             _ids;

 };

}

#endif //FTGRAPHICS_FT_MATERIAL_H
