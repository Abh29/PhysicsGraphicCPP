#include <utility>

#include "../includes/ft_material.h"

ft::Material::Material(Device::pointer device, Sampler::pointer sampler, std::string texture):
_id(ft::Material::ID()), _ftDevice(std::move(device)), _ftSampler(std::move(sampler)), _path(std::move(texture)){
    createTextureResource();
}

void ft::Material::createTextureResource() {
    // load the image file
    int texWidth, texHeight, texChannels;
    stbi_uc *pixels = stbi_load(_path.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
    VkDeviceSize imageSize = texWidth * texHeight * 4;

    uint32_t mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(texWidth, texHeight)))) + 1;

    if (!pixels) {
        throw std::runtime_error("failed to load texture image!");
    }

    // staging buffer and memory
    ft::BufferBuilder bufferBuilder;
    ft::ImageBuilder imageBuilder;
    auto stagingBuffer = bufferBuilder.setSize(imageSize)
            .setUsageFlags(VK_BUFFER_USAGE_TRANSFER_SRC_BIT)
            .setMemoryProperties(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)
            .setIsMapped(true)
            .build(_ftDevice);

    stagingBuffer->copyToMappedData(pixels, imageSize);

    // create texture image on device with memory
    _ftTextureImage = imageBuilder.setWidthHeight(texWidth, texHeight)
            .setMipLevel(mipLevels)
            .setSampleCount(VK_SAMPLE_COUNT_1_BIT)
            .setFormat(VK_FORMAT_R8G8B8A8_SRGB)
            .setTiling(VK_IMAGE_TILING_OPTIMAL)
            .setUsageFlags(VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT)
            .setMemoryProperties(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
            .setAspectFlags(VK_IMAGE_ASPECT_COLOR_BIT)
            .build(_ftDevice);

    // transition the image layout for dst copy
    Image::transitionImageLayout(_ftDevice, _ftTextureImage->getVKImage(), VK_FORMAT_R8G8B8A8_SRGB,
                                 VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, mipLevels);

    // copy the buffer to the image
    stagingBuffer->copyToImage(_ftTextureImage, texWidth, texHeight);

    // generate mip maps
//	generateMipmaps(_ftTextureImage->getVKImage(), VK_FORMAT_R8G8B8A8_SRGB, texWidth, texHeight, mipLevels);
    _ftTextureImage->generateMipmaps(VK_FORMAT_R8G8B8A8_SRGB);
    // clean up
    stbi_image_free(pixels);
}

ft::Image::pointer ft::Material::getTextureImage() {return _ftTextureImage;}

uint32_t ft::Material::getID() const {return _id;}

uint32_t ft::Material::ID() {
    static uint32_t  id = 1;
    return id++;
}

std::string ft::Material::getTexturePath() const {return _path;}

bool ft::Material::operator==(const ft::Material &m) const {return _path == m._path;}

/***************************************MaterialPool*************************************************/

ft::MaterialPool::MaterialPool(Device::pointer device): _ftDevice(std::move(device)) {}

ft::Material::pointer ft::MaterialPool::createMaterial(std::string path, Sampler::pointer sampler) {
    if (_textures.find(path) != _textures.end()) return _textures[path];

    Material::pointer m;

    try {
        m = std::make_shared<ft::Material>(_ftDevice, sampler, path);
    } catch (std::exception& e) {
        return nullptr;
    }
    _textures.insert(std::make_pair(path, m));
    return m;
}

