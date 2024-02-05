#include "../includes/ft_model.h"

ft::Model::Model(Device::pointer device,
				 std::string filePath, uint32_t bufferCount) :
		_ftDevice(device), _modelPath(filePath), _hasIndices(false) {
	_ids[0] = ft::Model::ID();
	_copiesCount = 1;
	_flags.fill(0);
	loadModel();
	createVertexBuffer();
	createIndexBuffer();

	BufferBuilder bufferBuilder;
	for (uint32_t i = 0; i < bufferCount; ++i) {
		_ftInstanceBuffers.push_back(bufferBuilder.setSize(sizeof(ft::PointLightObject) * ft::POINT_LIGHT_MAX_COUNT)
												.setUsageFlags(VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT)
												.setMemoryProperties(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)
												.setIsMapped(true)
												.setMappedOffset(0)
												.setMappedFlags(0)
												.build(_ftDevice)
		);
	}
}

ft::Model::Model(Device::pointer device, const tinygltf::Model& gltfInput, const tinygltf::Node &inputNode, uint32_t bufferCount) :
        _ftDevice(std::move(device)), _hasIndices(false) {
    _ids[0] = ft::Model::ID();
    _copiesCount = 1;
    _flags.fill(0);

    loadNode(inputNode, gltfInput, nullptr);
    createVertexBuffer();
    createIndexBuffer();

    BufferBuilder bufferBuilder;
    for (uint32_t i = 0; i < bufferCount; ++i) {
        _ftInstanceBuffers.push_back(bufferBuilder.setSize(sizeof(ft::PointLightObject) * ft::POINT_LIGHT_MAX_COUNT)
                                             .setUsageFlags(VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT)
                                             .setMemoryProperties(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)
                                             .setIsMapped(true)
                                             .setMappedOffset(0)
                                             .setMappedFlags(0)
                                             .build(_ftDevice)
        );
    }
}

ft::Model::~Model() {
    delete _node;
}

void ft::Model::bind(ft::CommandBuffer::pointer commandBuffer, uint32_t index) {
	writePerInstanceData(index);
	VkBuffer vertexBuffers[] = {_ftVertexBuffer->getVKBuffer(), _ftInstanceBuffers[index]->getVKBuffer()};
	VkDeviceSize offsets[] = {0, 0};
	vkCmdBindVertexBuffers(commandBuffer->getVKCommandBuffer(), 0, 2, vertexBuffers, offsets);
	if (_hasIndices)
		vkCmdBindIndexBuffer(commandBuffer->getVKCommandBuffer(), _ftIndexBuffer->getVKBuffer(), 0, VK_INDEX_TYPE_UINT32);
}

void ft::Model::draw(ft::CommandBuffer::pointer commandBuffer) {
	if (_hasIndices) {
        drawNode(commandBuffer, _node);
//		vkCmdDrawIndexed(commandBuffer->getVKCommandBuffer(), static_cast<uint32_t>(_indices.size()), _copiesCount, 0, 0, 0);
    } else
		vkCmdDraw(commandBuffer->getVKCommandBuffer(), static_cast<uint32_t>(_indices.size()), _copiesCount, 0, 0);
}

uint32_t ft::Model::addCopy(const ft::InstanceData &copyData) {
	uint32_t id = ID();
	_ids[_copiesCount] = id;
	_copies[_copiesCount] = copyData;
//	_copies[_copiesCount].id = Model::uint32ToVec3(id);
	_copies[_copiesCount].id = id;
	++_copiesCount;
	return id;
}

void ft::Model::loadModel() {
	tinyobj::attrib_t	attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	std::string warn, err;

	if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, _modelPath.c_str())) {
		throw std::runtime_error(warn + err);
	}

	std::unordered_map<Vertex, uint32_t> uniqueVertices{};

	for (const auto& shape: shapes) {
		for (const auto& index: shape.mesh.indices) {
			Vertex vertex{};

			vertex.pos = {
					attrib.vertices[3 * index.vertex_index + 0],
					attrib.vertices[3 * index.vertex_index + 1],
					attrib.vertices[3 * index.vertex_index + 2],
			};

            if (index.normal_index >= 0)
                vertex.normal = {
                        attrib.normals[3 * index.normal_index + 0],
                        attrib.normals[3 * index.normal_index + 1],
                        attrib.normals[3 * index.normal_index + 2],
                };

			if (index.texcoord_index >= 0)
				vertex.texCoord =  {
						attrib.texcoords[2 * index.texcoord_index + 0],
						1.0f - attrib.texcoords[2 * index.texcoord_index + 1],
				};

			vertex.color = {1.0f, 1.0f, 1.0f};

			if (uniqueVertices.count(vertex) == 0) {
				uniqueVertices[vertex] = static_cast<uint32_t>(_vertices.size());
				_vertices.push_back(vertex);
			}

			_indices.push_back(uniqueVertices[vertex]);
		}
	}
	_hasIndices = true;

    _node = new Node();
    _node->parent = nullptr;
    _node->mesh.push_back({0, static_cast<uint32_t>(_indices.size()), 0});
    _node->name = _modelPath;
    _node->visible = true;
    _node->id = _ids[0];

    _allNodes.insert(std::make_pair(_node->id, _node));

    std::cout << "model: " <<  _modelPath << " has: " << _indices.size() << " vertices." << std::endl;
}

void ft::Model::writePerInstanceData(uint32_t index) {
	_ftInstanceBuffers[index]->copyToMappedData(_copies.data(), sizeof(InstanceData) * _copiesCount);
}

void ft::Model::createVertexBuffer() {
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

void ft::Model::createIndexBuffer() {
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

void ft::Model::loadNode(const tinygltf::Node &inputNode, const tinygltf::Model &gltfInput, ft::Node *parent) {
    auto *node = new Node();
    node->name = inputNode.name;
    node->parent = parent;
    node->id = Model::ID();

    node->matrix = glm::mat4(1.0f);
    if (inputNode.translation.size() == 3) {
        node->matrix = glm::translate(node->matrix, glm::vec3(glm::make_vec3(inputNode.translation.data())));
    }
    if (inputNode.rotation.size() == 4) {
        glm::quat q = glm::make_quat(inputNode.rotation.data());
        node->matrix *= glm::mat4(q);
    }
    if (inputNode.scale.size() == 3){
        node->matrix = glm::scale(node->matrix, glm::vec3(glm::make_vec3(inputNode.scale.data())));
    }
    if (inputNode.matrix.size() == 16){
        node->matrix = glm::make_mat4x4(inputNode.matrix.data());
    }

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

            Primitive primitive{};
            primitive.firstIndex = firstIndex;
            primitive.indexCount = indexCount;
            primitive.materialIndex = p.material;
            node->mesh.push_back(primitive);
            _hasIndices = true;
        }
    }

    if (parent) {
        parent->children.push_back(node);
    } else {
        _node = node;
    }

    _allNodes.insert(std::make_pair(_node->id, _node));

}

uint32_t ft::Model::ID() {
	static uint32_t  id = 1;
	return id++;
}

ft::InstanceData* ft::Model::getInstanceData(uint32_t id) {
	for (size_t i = 0; i < _copiesCount ; ++i) {
		if (_ids[i] == id)
			 return std::addressof(_copies[i]);
	}
	return nullptr;
}

bool ft::Model::setInstanceData(InstanceData &copyData, uint32_t id) {
	for (size_t i = 0; i < _copiesCount ; ++i) {
		if (_ids[i] == id) {
			_copies[i] =  copyData;
			return true;
		}
	}
	return false;
}

bool ft::Model::findID(uint32_t id) const {
	for (size_t i = 0; i < _copiesCount ; ++i) {
		if (_ids[i] == id)
			return true;
	}
	return false;
}

uint32_t ft::Model::getFirstId() const {return _ids[0];}
std::array<uint32_t, 100> &ft::Model::getIds() {return _ids;}

std::array<ft::InstanceData, 100>& ft::Model::getCopies() {return _copies;}

bool ft::Model::select(uint32_t id) {
	for (size_t i = 0; i < _copiesCount ; ++i) {
		if (_ids[i] == id) {
            if (!(_flags[i] & ft::MODEL_SELECTED_BIT)) {
                std::cout << "selecting " << std::endl;
                _flags[i] |= ft::MODEL_SELECTED_BIT;
                _copies[i].color += glm::vec3 {0.7, 0.2, 0.05f};
                _copies[i].model = glm::scale(_copies[i].model, {1.25f, 1.25f, 1.25f});
            } else {
                std::cout << "unselecting " << std::endl;
                _flags[i] &= ~ft::MODEL_SELECTED_BIT;
                _copies[i].color -= glm::vec3 {0.7, 0.2, 0.05f};
                _copies[i].model = glm::scale(_copies[i].model, {0.8f, 0.8f, 0.8f});

            }
			return true;
		}
	}
	return false;
}
bool ft::Model::unselect(uint32_t id) {
	for (size_t i = 0; i < _copiesCount ; ++i) {
		if (_ids[i] == id) {
			_flags[i] &= ~ft::MODEL_SELECTED_BIT;
			return true;
		}
	}
	return false;
}
bool ft::Model::isSelected(uint32_t id) const {
	for (size_t i = 0; i < _copiesCount ; ++i) {
		if (_ids[i] == id)
			return _flags[i] & ft::MODEL_SELECTED_BIT;
	}
	return false;
}

void ft::Model::selectAll() {
	std::for_each(_flags.begin(), _flags.end(), [](uint32_t &flag) {
		flag |= ft::MODEL_SELECTED_BIT;
	});
}
void ft::Model::unselectAll() {
//	std::for_each(_flags.begin(), _flags.end(), [](uint32_t &flag) {
//		flag &= ~ft::MODEL_SELECTED_BIT;
//	});

    for (unsigned long i = 0; i < _copies.size(); ++i) {
        if (_flags[i] & ft::MODEL_SELECTED_BIT) {
            _flags[i] &= ~ft::MODEL_SELECTED_BIT;
            _copies[i].color -= glm::vec3 {0.7, 0.2, 0.05f};
            _copies[i].model = glm::scale(_copies[i].model, {0.8f, 0.8f, 0.8f});
        }
    }
}

glm::vec3 ft::Model::uint32ToVec3(uint32_t value)  {
	// Extract R, G, B components from the uint32 value
	uint8_t r = (value >> 16) & 0xFF; // Red component
	uint8_t g = (value >> 8) & 0xFF;  // Green component
	uint8_t b = value & 0xFF;         // Blue component

	// Normalize each component to the range [0, 1]
	float normR = static_cast<float>(r) / 255.0f;
	float normG = static_cast<float>(g) / 255.0f;
	float normB = static_cast<float>(b) / 255.0f;

	// Create a vec3 from the normalized components
	return glm::vec3(normR, normG, normB);
}

uint32_t ft::Model::vec3ToUint32(glm::vec3 &v) {

	// Scale the color components to [0, 255] range and convert to uint8_t
	uint8_t r = static_cast<uint8_t>(v.r * 255.0f);
	uint8_t g = static_cast<uint8_t>(v.g * 255.0f);
	uint8_t b = static_cast<uint8_t>(v.b * 255.0f);

	// Pack R, G, B components into a single uint32_t value
	uint32_t packedColor = (r << 16) | (g << 8) | b;
	return packedColor;
}

void ft::Model::overrideFlags(uint32_t id, uint32_t flags) {
    for (size_t i = 0; i < _copiesCount ; ++i) {
        if (_ids[i] == id) {
            _flags[i] = flags;
            break;
        }
    }
}

void ft::Model::setFlags(uint32_t id, uint32_t flags) {
    for (size_t i = 0; i < _copiesCount ; ++i) {
        if (_ids[i] == id) {
            _flags[i] |= flags;
            break;
        }
    }
}

void ft::Model::unsetFlags(uint32_t id, uint32_t flags) {
    for (size_t i = 0; i < _copiesCount ; ++i) {
        if (_ids[i] == id) {
            _flags[i] &= ~flags;
            break;
        }
    }
}

void ft::Model::setMaterial(uint32_t materialID, uint32_t nodeID) {
    if (nodeID == 0)
        _node->mesh[0].materialIndex = materialID;
    //todo: add an else
    std::for_each(_flags.begin(), _flags.end(), [](uint32_t& flag) {
        flag |= ft::MODEL_TEXTURED_BIT;
        flag |= ft::MODEL_HAS_COLOR_TEXTURE_BIT;
        flag |= ft::MODEL_HAS_NORMAL_TEXTURE_BIT;
    });
}

void ft::Model::unsetMaterial() {
    std::for_each(_flags.begin(), _flags.end(), [](uint32_t& flag) {
        flag &= ~ft::MODEL_TEXTURED_BIT;
        flag &= ~ft::MODEL_HAS_COLOR_TEXTURE_BIT;
        flag &= ~ft::MODEL_HAS_NORMAL_TEXTURE_BIT;
    });
}

bool ft::Model::hasMaterial() {
    return (_flags[0] & ft::MODEL_HAS_COLOR_TEXTURE_BIT);
}

void ft::Model::remapMaterials(std::map<uint32_t, uint32_t> idToMaterial, TexturePool::pointer pool) {
    (void) idToMaterial;
    (void) pool;
}

void ft::Model::drawNode(CommandBuffer::pointer commandBuffer, ft::Node *node) {
    if (!node->visible) return;
    for (const auto& p : node->mesh) {
        if (p.indexCount > 0) {
            vkCmdDrawIndexed(commandBuffer->getVKCommandBuffer(), p.indexCount, _copiesCount, p.firstIndex, 0, 0);
        }
    }
    for (auto& n : node->children)
        drawNode(commandBuffer, n);
}
