#include <utility>

#include "../includes/ft_scene.h"


ft::Scene::Scene(ft::Device::pointer device, std::vector<ft::Buffer::pointer> ubos):
_ftDevice(std::move(device)), _ftUniformBuffers(std::move(ubos)) {

	// point lights
	_ubo.pLCount = 0;

	// general lighting
	_generalLighting.lightColor = {1.0f, 1.0f, 1.0f};
	_generalLighting.lightDirection = {10.0f, 10.0f, 10.0f};
	_generalLighting.ambient = 0.2f;
}

void ft::Scene::drawSimpleObjs(CommandBuffer::pointer &commandBuffer, const GraphicsPipeline::pointer& pipeline, uint32_t index) {
	// push constant
	vkCmdPushConstants(commandBuffer->getVKCommandBuffer(), pipeline->getVKPipelineLayout(), VK_SHADER_STAGE_VERTEX_BIT,
					   0, static_cast<uint32_t>(sizeof(_generalLighting)), &_generalLighting);

	_ftUniformBuffers[index]->copyToMappedData(&_ubo, sizeof(_ubo));
	// vertex and index buffers
	for (auto& model : _models) {
        if (model->hasMaterial()) continue;
		model->bind(commandBuffer, index);
		model->draw(commandBuffer);
	}
}

void ft::Scene::drawTexturedObjs(ft::CommandBuffer::pointer commandBuffer, ft::GraphicsPipeline::pointer pipeline, ft::TexturedRdrSys::pointer system,
                                 uint32_t index) {

    // bind the graphics pipeline
    vkCmdBindPipeline(commandBuffer->getVKCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, system->getGraphicsPipeline()->getVKPipeline());

    // push constant
    vkCmdPushConstants(commandBuffer->getVKCommandBuffer(), pipeline->getVKPipelineLayout(), VK_SHADER_STAGE_VERTEX_BIT,
                       0, static_cast<uint32_t>(sizeof(_generalLighting)), &_generalLighting);

    _ftUniformBuffers[index]->copyToMappedData(&_ubo, sizeof(_ubo));

    // vertex and index buffers
    for (auto & i : _materialToModel) {
        auto mat = _ftTexturePool->getTextureByID(i.first);
        system->populateUBODescriptors(_ftUniformBuffers, mat);
        system->populateTextureDescriptors(mat);
//        mat->getDescriptorSet(index)->updateDescriptorBuffer()
        // bind the descriptor sets
        vkCmdBindDescriptorSets(commandBuffer->getVKCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS,
                                system->getGraphicsPipeline()->getVKPipelineLayout(), 0, 1,
                                &(mat->getDescriptorSet(index)->getVKDescriptorSet()),
                                0, nullptr);
        for (auto& model : i.second) {
            model->bind(commandBuffer, index);
            model->draw(commandBuffer);
        }
    }
}

uint32_t ft::Scene::addModelFromObj(std::string objectPath, ft::InstanceData data) {
	Model::pointer model = std::make_shared<Model>(_ftDevice, objectPath, _ftUniformBuffers.size());
	data.normalMatrix = glm::inverseTranspose(data.model * _ubo.view);
//	data.id = Model::uint32ToVec3(model->getFirstId());
	data.id = model->getFirstId();
	model->getCopies()[0] = data;
	_models.push_back(model);
	return model->getFirstId();
}

uint32_t ft::Scene::addModelsFromGltf(std::string gltfPath, DescriptorPool::pointer pool,
                                      DescriptorSetLayout::pointer layout) {
    tinygltf::Model gltfInput;
    tinygltf::TinyGLTF gltfContext;
    std::string error, warning;

    if (!gltfContext.LoadASCIIFromFile(&gltfInput, &error, &warning, gltfPath))
        throw std::runtime_error("Could not open GLTF file!\n" + error + "\n" + warning);

    std::string path = gltfPath.substr(0, gltfPath.find_last_of('/'));
    std::map<uint32_t , uint32_t> id2texture;

    // load images
    for (size_t i = 0; i < gltfInput.images.size(); ++i) {
        auto t = _ftTexturePool->createTexture(path + "/" + gltfInput.images[i].uri, pool, layout, ft::Texture::FileType::FT_TEXTURE_KTX);
        id2texture.insert(std::make_pair(i, t->getID()));
    }

    std::cout << "images size: " << gltfInput.images.size() << std::endl;
    std::cout << "materials size: " << gltfInput.materials.size() << std::endl;

    // load materials
    std::map<uint32_t, uint32_t> id2Material;
    for (size_t i = 0; i < gltfInput.materials.size(); ++i) {
        Material material{};
        tinygltf::Material m = gltfInput.materials[i];
        // base color factor
        if (m.values.find("baseColorFactor") != m.values.end()) {
            material.colorFactor = glm::make_vec4(m.values["baseColorFactor"].ColorFactor().data());
        }
        // base color texture index
        if (m.values.find("baseColorTexture") != m.values.end()) {
            material.colorTextureIndex = m.values["baseColorTexture"].TextureIndex();
        }
        // normal map texture index
        if (m.additionalValues.find("normalTexture") != m.additionalValues.end()) {
            material.normalTextureIndex = m.additionalValues["normalTexture"].TextureIndex();
        }

        material.alphaModel = m.alphaMode;
        material.alphaCutOff = (float)m.alphaCutoff;
        material.doubleSided = m.doubleSided;
        _ftTexturePool->addMaterial(material);
    }


    // load textures
    std::vector<uint32_t> textures(gltfInput.textures.size());
    for (size_t i = 0; i < gltfInput.textures.size(); ++i) {
        textures[i] = id2texture[gltfInput.textures[i].source];
    }

    // load gltf scene
    tinygltf::Scene& scene = gltfInput.scenes[0];
    for (size_t i = 0; i < scene.nodes.size(); ++i) {
        const tinygltf::Node node = gltfInput.nodes[scene.nodes[i]];
        Model::pointer model = std::make_shared<Model>(_ftDevice, gltfInput, node, _ftUniformBuffers.size());
        ft::InstanceData data{};
        data.normalMatrix = glm::inverseTranspose(data.model * _ubo.view);
        data.id = model->getFirstId();
        model->getCopies()[0] = data;
        _models.push_back(model);
    }

    return -1;
}

void ft::Scene::addPointLightToTheScene(PointLightObject &pl) {
	_ubo.lights[_ubo.pLCount++] = pl;
}

uint32_t ft::Scene::addObjectCopyToTheScene(uint32_t id, ft::InstanceData data) {
	data.normalMatrix = glm::inverseTranspose(data.model * _ubo.view);
	for (auto& m : _models) {
		if (m->findID(id))
			return m->addCopy(data);
	}
	return -1;
}

ft::Camera::pointer ft::Scene::getCamera() const {
	return _camera;
}

void ft::Scene::setCamera(ft::Camera::pointer camera) {
	_camera = std::move(camera);
	_ubo.view = _camera->getViewMatrix();
	_ubo.proj = _camera->getProjMatrix();
}

void ft::Scene::setGeneralLight(glm::vec3 color, glm::vec3 direction, float ambient) {
	_generalLighting.lightColor = color;
	_generalLighting.lightDirection = direction;
	_generalLighting.ambient = ambient;
}

void ft::Scene::updateCameraUBO() {
	_ubo.view = _camera->getViewMatrix();
	_ubo.proj = _camera->getProjMatrix();
}

ft::PushConstantObject &ft::Scene::getGeneralLighting() {return _generalLighting;}

ft::PointLightObject* ft::Scene::getLights() {return _ubo.lights;}

std::vector<ft::Model::pointer> ft::Scene::getModels() const {return _models;}


/*
//
//void ft::Scene::addTextureToObject(uint32_t id, Texture::pointer material) {
//    for (auto& m : _models) {
//        if (m->findID(id)) {
//            m->setMaterial(material);
//            if (_materialToModel.find(material->getID()) == _materialToModel.end()) {
//                _materialToModel[material->getID()] = {m};
//            } else {
//                _materialToModel[material->getID()].push_back(m);
//            }
//            break;
//        }
//    }
//}
*/


void ft::Scene::setMaterialPool(TexturePool::pointer pool) { _ftTexturePool = std::move(pool);}

void
ft::Scene::drawPickableObjs(const ft::CommandBuffer::pointer &commandBuffer, const ft::GraphicsPipeline::pointer &pipeline,
                            uint32_t index) {
    (void) pipeline;
    // vertex and index buffers
    for (auto& model : _models) {
        model->bind(commandBuffer, index);
        model->draw(commandBuffer);
    }
}


bool ft::Scene::select(uint32_t id) {
    for (auto& m : _models) {
        if (m->findID(id)) {
            return m->select(id);
        }
    }
    return false;
}

void ft::Scene::unselectAll() {
    for (auto& m : _models) {
      m->unselectAll();
    }
}

void ft::GLTFScene::loadGLTFFile(std::string filename, ft::DescriptorPool::pointer pool, ft::DescriptorSetLayout::pointer layout) {
    tinygltf::Model gltfInput;
    tinygltf::TinyGLTF gltfContext;
    std::string error, warning;


    if (!gltfContext.LoadASCIIFromFile(&gltfInput, &error, &warning, filename))
        throw std::runtime_error("Could not open GLTF file!\n" + error + "\n" + warning);

    std::cout << "images: " << gltfInput.images.size() << std::endl;
    std::cout << "textures: " << gltfInput.textures.size() << std::endl;
    std::cout << "materials: " << gltfInput.materials.size() << std::endl;


    std::string path = filename.substr(0, filename.find_last_of('/'));
    std::map<uint32_t , uint32_t> id2id;
    auto sampler = std::make_shared<ft::Sampler>(_ftDevice);

    // load images
    for (size_t i = 0; i < gltfInput.images.size(); ++i) {
        std::cout << path + "/" + gltfInput.images[i].uri << std::endl;
        auto t = _ftTexturePool->createTexture(path + "/" + gltfInput.images[i].uri, pool, layout, ft::Texture::FileType::FT_TEXTURE_KTX);
        _images.push_back(t);
        id2id.insert(std::make_pair(i, t->getID()));
    }

    // load materials
    _materials.resize(gltfInput.materials.size());
    for (size_t i = 0; i < gltfInput.materials.size(); ++i) {
        tinygltf::Material m = gltfInput.materials[i];
        // base color factor
        if (m.values.find("baseColorFactor") != m.values.end()) {
            _materials[i].colorFactor = glm::make_vec4(m.values["baseColorFactor"].ColorFactor().data());
        }
        // base color texture index
        if (m.values.find("baseColorTexture") != m.values.end()) {
            _materials[i].colorTextureIndex = m.values["baseColorTexture"].TextureIndex();
        }
        // normal map texture index
        if (m.additionalValues.find("normalTexture") != m.additionalValues.end()) {
            _materials[i].normalTextureIndex = m.additionalValues["normalTexture"].TextureIndex();
        }
        _materials[i].alphaModel = m.alphaMode;
        _materials[i].alphaCutOff = (float)m.alphaCutoff;
        _materials[i].doubleSided = m.doubleSided;
    }

    // load textures
    _textures.resize(gltfInput.textures.size());
    for (size_t i = 0; i < gltfInput.textures.size(); ++i) {
        _textures[i].imageIndex = gltfInput.textures[i].source;
    }

    // load gltf scene
    tinygltf::Scene& scene = gltfInput.scenes[0];
    for (size_t i = 0; i < scene.nodes.size(); ++i) {
        const tinygltf::Node node = gltfInput.nodes[scene.nodes[i]];
        loadNode(node, gltfInput, nullptr);
    }

    // create vertex and index buffers
    createVertexBuffer();
    createIndexBuffer();

}

void ft::GLTFScene::setMaterialPool(TexturePool::pointer pool) {
    _ftTexturePool = std::move(pool);
}

void ft::GLTFScene::loadNode(const tinygltf::Node &inputNode, const tinygltf::Model &gltfInput,
                             ft::GLTFScene::Node_ *parent) {
    auto *node = new Node_();
    node->name = inputNode.name;
    node->parent = parent;

    node->matrix = glm::mat4(1.0f);
    if (inputNode.translation.size() == 3)
        node->matrix = glm::translate(node->matrix, glm::vec3(glm::make_vec3(inputNode.translation.data())));
    if (inputNode.rotation.size() == 4) {
        glm::quat q = glm::make_quat(inputNode.rotation.data());
        node->matrix *= glm::mat4(q);
    }
    if (inputNode.scale.size() == 3)
        node->matrix = glm::scale(node->matrix, glm::vec3(glm::make_vec3(inputNode.scale.data())));
    if (inputNode.matrix.size() == 16)
        node->matrix = glm::make_mat4x4(inputNode.matrix.data());

    // load children nodes
    if (inputNode.children.size() > 0) {
        for (size_t i = 0; i < inputNode.children.size(); ++i) {
            loadNode(gltfInput.nodes[inputNode.children[i]], gltfInput, node);
        }
    }

    // load mesh data
    if (inputNode.mesh > -1) {
        const tinygltf::Mesh mesh = gltfInput.meshes[inputNode.mesh];
        for (size_t i = 0; i < mesh.primitives.size(); ++i) {
            const auto& p = mesh.primitives[i];
            uint32_t firstIndex = static_cast<uint32_t>(_indices.size());
            uint32_t vertexStart = static_cast<uint32_t>(_vertices.size());
            uint32_t indexCount = 0;

            // vertices
            const float* positionBuffer = nullptr;
            const float* normalsBuffer = nullptr;
            const float* texCoordsBuffer = nullptr;
            const float* tangentsBuffer = nullptr;
            size_t vertexCount = 0;

            if (p.attributes.find("POSITION") != p.attributes.end()) {
                const tinygltf::Accessor& accessor = gltfInput.accessors[p.attributes.find("POSITION")->second];
                const tinygltf::BufferView& view = gltfInput.bufferViews[accessor.bufferView];
                positionBuffer = reinterpret_cast<const float*>(&(gltfInput.buffers[view.buffer].data[accessor.byteOffset + view.byteOffset]));
                vertexCount = accessor.count;
            }
            if (p.attributes.find("NORMAL") != p.attributes.end()) {
                const tinygltf::Accessor& accessor = gltfInput.accessors[p.attributes.find("NORMAL")->second];
                const tinygltf::BufferView& view = gltfInput.bufferViews[accessor.bufferView];
                normalsBuffer = reinterpret_cast<const float*>(&(gltfInput.buffers[view.buffer].data[accessor.byteOffset + view.byteOffset]));

            }
            if (p.attributes.find("TEXCOORD_0") != p.attributes.end()) {
                const tinygltf::Accessor& accessor = gltfInput.accessors[p.attributes.find("TEXCOORD_0")->second];
                const tinygltf::BufferView& view = gltfInput.bufferViews[accessor.bufferView];
                texCoordsBuffer = reinterpret_cast<const float*>(&(gltfInput.buffers[view.buffer].data[accessor.byteOffset + view.byteOffset]));

            }
            if (p.attributes.find("TANGENT") != p.attributes.end()) {
                const tinygltf::Accessor& accessor = gltfInput.accessors[p.attributes.find("TAGENT")->second];
                const tinygltf::BufferView& view = gltfInput.bufferViews[accessor.bufferView];
                tangentsBuffer = reinterpret_cast<const float*>(&(gltfInput.buffers[view.buffer].data[accessor.byteOffset + view.byteOffset]));
            }

            for (size_t j = 0; j < vertexCount; ++j) {
                Vertex v{};
                v.pos = glm::make_vec3(&positionBuffer[j * 3]);
                if (normalsBuffer)
                    v.normal = glm::make_vec3(&normalsBuffer[j * 3]);
                if (texCoordsBuffer)
                    v.texCoord = glm::make_vec2(&texCoordsBuffer[j * 2]);
                v.color = glm::vec3(1.0f);
                if (tangentsBuffer)
                    v.tangent = glm::make_vec4(&tangentsBuffer[j * 4]);
                _vertices.push_back(v);
            }

            // indices
            const tinygltf::Accessor& accessor = gltfInput.accessors[p.indices];
            const tinygltf::BufferView& bufferView = gltfInput.bufferViews[accessor.bufferView];
            const tinygltf::Buffer& buffer = gltfInput.buffers[bufferView.buffer];

            indexCount += static_cast<uint32_t>(accessor.count);
            switch (accessor.componentType) {
                case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT: {
                    auto buff = reinterpret_cast<const uint32_t*>(&buffer.data[accessor.byteOffset + bufferView.byteOffset]);
                    for (size_t j = 0; j < accessor.count; ++j) {
                        _indices.push_back(buff[j] + vertexStart);
                    }
                    break;
                }
                case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT: {
                    auto buff = reinterpret_cast<const uint16_t*>(&buffer.data[accessor.byteOffset + bufferView.byteOffset]);
                    for (size_t j = 0; j < accessor.count; ++j) {
                        _indices.push_back(buff[j] + vertexStart);
                    }
                    break;
                }
                case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE: {
                    auto buff = reinterpret_cast<const uint8_t*>(&buffer.data[accessor.byteOffset + bufferView.byteOffset]);
                    for (size_t j = 0; j < accessor.count; ++j) {
                        _indices.push_back(buff[j] + vertexStart);
                    }
                    break;
                }
                default:
                    //todo: do better
                    return;
            }

            Primitive_ primitive{};
            primitive.firstIndex = firstIndex;
            primitive.indexCount = indexCount;
            primitive.materialIndex = p.material;
            node->mesh.primitives.push_back(primitive);
        }
    }

    std::cout << "indices: " << _indices.size() << std::endl;
    std::cout << "vertices: " << _vertices.size() << std::endl;

    if (parent) {
        parent->children.push_back(node);
    } else {
        _nodes.push_back(node);
    }
}

void ft::GLTFScene::createVertexBuffer() {
    // create a staging buffer with memory
    VkDeviceSize bufferSize = sizeof(_vertices[0]) * _vertices.size();
    BufferBuilder bufferBuilder;

    auto stagingBuffer = bufferBuilder.setSize(bufferSize)
            .setUsageFlags(VK_BUFFER_USAGE_TRANSFER_SRC_BIT)
            .setMemoryProperties(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)
            .setIsMapped(true)
            .build(_ftDevice);

    // fill the staging vertex buffer
    stagingBuffer->copyToMappedData(_vertices.data(), bufferSize);


    // create a dest buffer
    _ftVertexBuffer = bufferBuilder.setSize(bufferSize)
            .setUsageFlags(VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT)
            .setMemoryProperties(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
            .build(_ftDevice);

    // copy the data
    stagingBuffer->copyToBuffer(_ftVertexBuffer, bufferSize);
}

void ft::GLTFScene::createIndexBuffer() {
    VkDeviceSize bufferSize = sizeof(_indices[0]) * _indices.size();
    BufferBuilder bufferBuilder;

    // create a staging buffer
    auto stagingBuffer = bufferBuilder.setSize(bufferSize)
            .setUsageFlags(VK_BUFFER_USAGE_TRANSFER_SRC_BIT)
            .setMemoryProperties(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)
            .setIsMapped(true)
            .build(_ftDevice);

    // write the data to the staging buffer
    stagingBuffer->copyToMappedData(_indices.data(), bufferSize);

    // create the index buffer
    _ftIndexBuffer = bufferBuilder.setSize(bufferSize)
            .setUsageFlags(VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT)
            .setMemoryProperties(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
            .setIsMapped(false)
            .build(_ftDevice);

    // copy buffer
    stagingBuffer->copyToBuffer(_ftIndexBuffer, bufferSize);
}

void ft::GLTFScene::draw(const CommandBuffer::pointer &commandBuffer, GraphicsPipeline::pointer pipeline,
                         TexturedRdrSys::pointer system, uint32_t frameIndex) {
    VkDeviceSize offsets[1] = {0};
    VkBuffer buffer[1] = {_ftVertexBuffer->getVKBuffer()};
    vkCmdBindVertexBuffers(commandBuffer->getVKCommandBuffer(), 0, 1, buffer, offsets);
    vkCmdBindIndexBuffer(commandBuffer->getVKCommandBuffer(), _ftIndexBuffer->getVKBuffer(), 0, VK_INDEX_TYPE_UINT32);
    for (auto& node : _nodes) {
        drawNode(node, commandBuffer, pipeline, system, frameIndex);
    }
}

void ft::GLTFScene::drawNode(Node_* node, const CommandBuffer::pointer &commandBuffer, GraphicsPipeline::pointer pipeline,
                             TexturedRdrSys::pointer system, uint32_t frameIndex) {
    if (!node->visible) return;
    if (node->mesh.primitives.size() > 0) {
        InstanceData data{};
        data.model = node->matrix;
        Node_* currentParent = node->parent;
        while (currentParent) {
            data.model *= currentParent->matrix;
            currentParent = currentParent->parent;
        }

    }
    for (auto& child : node->children) {
        drawNode(child, commandBuffer, pipeline, system, frameIndex);
    }
}
