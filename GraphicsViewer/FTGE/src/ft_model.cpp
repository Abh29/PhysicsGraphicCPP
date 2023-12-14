#include "../include.h"

ft::Model::Model(Device::pointer device, CommandPool::pointer commandPool,
				 std::string filePath, ft::Buffer::pointer instanceBuffer) :
		_ftDevice(device), _ftCommandPool(commandPool), _ftInstanceBuffer(instanceBuffer),
		_modelPath(filePath), _hasIndices(false) {
	_ids[0] = ft::Model::ID();
	_copiesCount = 1;
	loadModel();
	createVertexBuffer();
	createIndexBuffer();
}

void ft::Model::bind(ft::CommandBuffer::pointer commandBuffer) {
	VkBuffer vertexBuffers[] = {_ftVertexBuffer->getVKBuffer(), _ftInstanceBuffer->getVKBuffer()};
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

void ft::Model::addCopy(const ft::InstanceData &copyData) {
	_ids[_copiesCount] = ID();
	_copies[_copiesCount++] = copyData;
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

}

void ft::Model::writePerInstanceData() {
	_ftInstanceBuffer->copyToMappedData(_copies.data(), sizeof(InstanceData) * _copiesCount);
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
	stagingBuffer->copyToBuffer(_ftCommandPool, _ftVertexBuffer, bufferSize);
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
	stagingBuffer->copyToBuffer(_ftCommandPool, _ftIndexBuffer, bufferSize);
}

uint32_t ft::Model::ID() {
	static uint32_t  id = 0;
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

std::array<ft::InstanceData, 100>& ft::Model::getCopies() {return _copies;}

