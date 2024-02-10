#include "../includes/ft_texture.h"
#include "../includes/ft_tools.h"

ft::Texture::Texture(Device::pointer device, std::string texture, ft::Texture::FileType fileType):
        _id(ft::Texture::ID()), _ftDevice(std::move(device)), _path(std::move(texture)){

    if (!ft::tools::fileExists(_path))
        throw std::runtime_error("Could not load " + _path + ", make sure the file exists!");

    switch (fileType) {
        case ft::Texture::FileType::FT_TEXTURE_KTX:
            createTextureFromKTXFile();
            break;
        default:
            createTextureResource();
    }
    _ftSampler = std::make_shared<Sampler>(_ftDevice);
}

void ft::Texture::createTextureResource() {

    // load the image file
//    int texWidth, texHeight, texChannels;
//    stbi_uc *pixels = stbi_load(_path.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
//    VkDeviceSize imageSize = texWidth * texHeight * 4;
//
//    uint32_t mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(texWidth, texHeight)))) + 1;
//
//    if (!pixels) {
//        throw std::runtime_error("failed to load texture image!");
//    }
//
//    // staging buffer and memory
//    ft::BufferBuilder bufferBuilder;
//    ft::ImageBuilder imageBuilder;
//    auto stagingBuffer = bufferBuilder.setSize(imageSize)
//            .setUsageFlags(VK_BUFFER_USAGE_TRANSFER_SRC_BIT)
//            .setMemoryProperties(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)
//            .setIsMapped(true)
//            .build(_ftDevice);
//
//    stagingBuffer->copyToMappedData(pixels, imageSize);
//
//    // create texture image on device with memory
//    _ftTextureImage = imageBuilder.setWidthHeight(texWidth, texHeight)
//            .setMipLevel(mipLevels)
//            .setSampleCount(VK_SAMPLE_COUNT_1_BIT)
//            .setFormat(VK_FORMAT_R8G8B8A8_SRGB)
//            .setTiling(VK_IMAGE_TILING_OPTIMAL)
//            .setUsageFlags(VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT)
//            .setMemoryProperties(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
//            .setAspectFlags(VK_IMAGE_ASPECT_COLOR_BIT)
//            .build(_ftDevice);
//
//    // transition the image layout for dst copy
//    Image::transitionImageLayout(_ftDevice, _ftTextureImage->getVKImage(), VK_FORMAT_R8G8B8A8_SRGB,
//                                 VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, mipLevels);
//
//    // copy the buffer to the image
//    stagingBuffer->copyToImage(_ftTextureImage, texWidth, texHeight);
//
//    // generate mip maps
//    _ftTextureImage->generateMipmaps(VK_FORMAT_R8G8B8A8_SRGB);
//    // clean up
//    stbi_image_free(pixels);
}

void ft::Texture::createTextureFromKTXFile() {
    ktxTexture *ktxTexture;

    ktxResult result = ktxTexture_CreateFromNamedFile(_path.c_str(), KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT, &ktxTexture);

    if (result != KTX_SUCCESS)
        throw std::runtime_error("Could not load KTX file!");


    ktx_uint8_t *data = ktxTexture_GetData(ktxTexture);
    ktx_size_t textureSize = ktxTexture_GetDataSize(ktxTexture);
    uint32_t mipLevels = ktxTexture->numLevels;

    std::vector<size_t> mipOffsets;
    for (uint32_t i = 0; i < mipLevels; ++i) {
        ktx_size_t offset;
        ktxTexture_GetImageOffset(ktxTexture, i, 0, 0, &offset);
        mipOffsets.push_back(offset);
    }

    // staging buffer and memory
    ft::BufferBuilder bufferBuilder;
    ft::ImageBuilder imageBuilder;

    auto stagingBuffer = bufferBuilder.setSize(textureSize)
            .setUsageFlags(VK_BUFFER_USAGE_TRANSFER_SRC_BIT)
            .setMemoryProperties(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)
            .setIsMapped(true)
            .build(_ftDevice);

    stagingBuffer->copyToMappedData(data, textureSize);

    _ftTextureImage = imageBuilder.setWidthHeight(ktxTexture->baseWidth, ktxTexture->baseHeight)
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
    stagingBuffer->copyToImage(_ftTextureImage, ktxTexture->baseWidth, ktxTexture->baseHeight, mipLevels, mipOffsets.data());

    // generate mip maps
//	generateMipmaps(_ftTextureImage->getVKImage(), VK_FORMAT_R8G8B8A8_SRGB, texWidth, texHeight, mipLevels);
    _ftTextureImage->generateMipmaps(VK_FORMAT_R8G8B8A8_SRGB);
    // clean up
    ktxTexture_Destroy(ktxTexture);
}

ft::Image::pointer ft::Texture::getTextureImage() {return _ftTextureImage;}

uint32_t ft::Texture::getID() const {return _id;}

uint32_t ft::Texture::ID() {
    static uint32_t  id = 1;
    return id++;
}

std::string ft::Texture::getTexturePath() const {return _path;}

bool ft::Texture::operator==(const ft::Texture &m) const {return _path == m._path;}

ft::Sampler::pointer ft::Texture::getSampler() const { return _ftSampler;}

ft::DescriptorSet::pointer ft::Texture::getDescriptorSet(uint32_t index) const {
    if (_ftDescriptorSets.empty()) return nullptr;
    return _ftDescriptorSets[index];
}

void ft::Texture::createDescriptorSets(DescriptorSetLayout::pointer layout, DescriptorPool::pointer pool) {
    _ftDescriptorSets.resize(ft::MAX_FRAMES_IN_FLIGHT);
    for (uint32_t i = 0; i < ft::MAX_FRAMES_IN_FLIGHT; ++i) {
        _ftDescriptorSets[i] = pool->allocateSet(layout);
    }
}

std::vector<ft::DescriptorSet::pointer> &ft::Texture::getDescriptorSets() {return _ftDescriptorSets;}

void ft::Texture::setDescriptorSets(std::vector<DescriptorSet::pointer> descriptorSets) { _ftDescriptorSets = std::move(descriptorSets);}

/***************************************Material*************************************************/

ft::Material::Material(Device::pointer device) : _ftDevice(std::move(device)) {}

ft::DescriptorSet::pointer ft::Material::getDescriptorSet(uint32_t index) {
    assert(index < _ftDescriptorSets.size());
    return _ftDescriptorSets[index];
}

void ft::Material::addTexture(Texture::pointer texture) {
    _ftTextures.push_back(texture);
}

void ft::Material::createDescriptors(DescriptorPool::pointer pool, DescriptorSetLayout::pointer layout) {
    _ftDescriptorSets.resize(ft::MAX_FRAMES_IN_FLIGHT);

    for (uint32_t i = 0; i < ft::MAX_FRAMES_IN_FLIGHT; ++i)
        _ftDescriptorSets[i] = pool->allocateSet(layout);
}

void ft::Material::bindDescriptor(uint32_t frameIndex, uint32_t textureIndex, uint32_t binding) {
    assert(textureIndex < _ftTextures.size());
    _ftDescriptorSets[frameIndex]->updateDescriptorImage(binding, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                                         VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                                                         _ftTextures[textureIndex]->getTextureImage(),
                                                         _ftTextures[textureIndex]->getSampler());
}

void ft::Material::setColorFactor(glm::vec4 v) {_colorFactor = v;}

void ft::Material::setAlphaMode(std::string a) { _alphaMode = a;}

void ft::Material::setAlphaCutOff(float a) { _alphaCutOff = a;}

void ft::Material::setDoubleSided(bool v) { _doubleSided = v;}

glm::vec4 ft::Material::getColorFactor() const {return _colorFactor;}

std::string ft::Material::getAlphaMode() const {return _alphaMode;}

float ft::Material::getAlphaCutOff() const {return _alphaCutOff;}

bool ft::Material::isDoubleSided() const {return _doubleSided;}

uint32_t ft::Material::getTexturesSize() const {return _ftTextures.size();}

ft::Texture::pointer ft::Material::getTexture(uint32_t index) const {return _ftTextures[index];}

/***************************************TexturePool*************************************************/

ft::TexturePool::TexturePool(Device::pointer device): _ftDevice(std::move(device)) {}

ft::Texture::pointer ft::TexturePool::createTexture(std::string path, Texture::FileType fileType) {
    if (_textures.find(path) != _textures.end()) return _textures[path];

    Texture::pointer m;

    try {
        m = std::make_shared<ft::Texture>(_ftDevice, path, fileType);
    } catch (std::exception& e) {
        return nullptr;
    }
    _textures.insert(std::make_pair(path, m));
    _ids.insert(std::make_pair(m->getID(), m));
    return m;
}

ft::Texture::pointer ft::TexturePool::getTextureByID(uint32_t id) {return _ids[id];}

ft::Material::pointer ft::TexturePool::getMaterialByID(uint32_t id) {
    assert(id < _materials.size());
    return _materials[id];
}

uint32_t ft::TexturePool::addMaterial(ft::Material::pointer m) {
    _materials.push_back(m);
    return _materials.size();
}

