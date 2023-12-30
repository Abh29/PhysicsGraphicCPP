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

void ft::Model::bind(ft::CommandBuffer::pointer commandBuffer, uint32_t index) {
	writePerInstanceData(index);
	VkBuffer vertexBuffers[] = {_ftVertexBuffer->getVKBuffer(), _ftInstanceBuffers[index]->getVKBuffer()};
	VkDeviceSize offsets[] = {0, 0};
	vkCmdBindVertexBuffers(commandBuffer->getVKCommandBuffer(), 0, 2, vertexBuffers, offsets);
	if (_hasIndices)
		vkCmdBindIndexBuffer(commandBuffer->getVKCommandBuffer(), _ftIndexBuffer->getVKBuffer(), 0, VK_INDEX_TYPE_UINT32);
}

void ft::Model::draw(ft::CommandBuffer::pointer commandBuffer) {
	if (_hasIndices)
		vkCmdDrawIndexed(commandBuffer->getVKCommandBuffer(), static_cast<uint32_t>(_indices.size()), _copiesCount, 0, 0, 0);
	else
		vkCmdDraw(commandBuffer->getVKCommandBuffer(), static_cast<uint32_t>(_indices.size()), _copiesCount, 0, 0);
}

uint32_t ft::Model::addCopy(const ft::InstanceData &copyData) {
	uint32_t id = ID();
	_ids[_copiesCount] = id;
	_copies[_copiesCount] = copyData;
	_copies[_copiesCount].id = Model::uint32ToVec3(id);
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
			_flags[i] |= ft::MODEL_SELECTED_BIT;
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
	std::for_each(_flags.begin(), _flags.end(), [](uint32_t &flag) {
		flag &= ~ft::MODEL_SELECTED_BIT;
	});
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
