#include "../includes/ft_vertex.h"

ft::Vertex::Vertex(glm::vec3 p, glm::vec3 c, glm::vec3 n, glm::vec2 t) :
pos{p}, color{c}, normal{n}, texCoord{t} {}

VkVertexInputBindingDescription ft::Vertex::getBindingDescription() {
	VkVertexInputBindingDescription bindingDescription{};
	bindingDescription.binding = 0;
	bindingDescription.stride = sizeof(Vertex);
	bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	return bindingDescription;
}

std::array<VkVertexInputAttributeDescription, 4> ft::Vertex::getAttributeDescription() {
	std::array<VkVertexInputAttributeDescription, 4> attributeDescriptions{};
	attributeDescriptions[0].binding = 0;
	attributeDescriptions[0].location = 0;
	attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
	attributeDescriptions[0].offset = offsetof(Vertex, pos);

	attributeDescriptions[1].binding = 0;
	attributeDescriptions[1].location = 1;
	attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
	attributeDescriptions[1].offset = offsetof(Vertex, color);

	attributeDescriptions[2].binding = 0;
	attributeDescriptions[2].location = 2;
	attributeDescriptions[2].format = VK_FORMAT_R32G32B32_SFLOAT;
	attributeDescriptions[2].offset = offsetof(Vertex, normal);

	attributeDescriptions[3].binding = 0;
	attributeDescriptions[3].location = 3;
	attributeDescriptions[3].format = VK_FORMAT_R32G32_SFLOAT;
	attributeDescriptions[3].offset = offsetof(Vertex, texCoord);

	return attributeDescriptions;
}

bool ft::Vertex::operator==(const Vertex &other) const {
	return pos == other.pos && color == other.color && texCoord == other.texCoord;
}


size_t std::hash<ft::Vertex>::operator()(const ft::Vertex &vertex) const {
	return ((hash<glm::vec3>()(vertex.pos) ^ (hash<glm::vec3>()(vertex.color) << 1)) >> 1) ^ (hash<glm::vec2>()(vertex.texCoord) << 1);
}


VkVertexInputBindingDescription ft::InstanceData::getBindingDescription() {
	VkVertexInputBindingDescription bindingDescription{};
	bindingDescription.binding = 1;
	bindingDescription.stride = sizeof(ft::InstanceData);
	bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_INSTANCE;

	return bindingDescription;
}

std::array<VkVertexInputAttributeDescription, 10> ft::InstanceData::getAttributeDescription() {
	std::array<VkVertexInputAttributeDescription, 10> attributeDescriptions{};
	attributeDescriptions[0].binding = 1;
	attributeDescriptions[0].location = 4;
	attributeDescriptions[0].format = VK_FORMAT_R32G32B32A32_SFLOAT;
	attributeDescriptions[0].offset = offsetof(ft::InstanceData, model);

	attributeDescriptions[1].binding = 1;
	attributeDescriptions[1].location = 5;
	attributeDescriptions[1].format = VK_FORMAT_R32G32B32A32_SFLOAT;
	attributeDescriptions[1].offset = offsetof(ft::InstanceData, model) + 16;

	attributeDescriptions[2].binding = 1;
	attributeDescriptions[2].location = 6;
	attributeDescriptions[2].format = VK_FORMAT_R32G32B32A32_SFLOAT;
	attributeDescriptions[2].offset = offsetof(ft::InstanceData, model) + 32;

	attributeDescriptions[3].binding = 1;
	attributeDescriptions[3].location = 7;
	attributeDescriptions[3].format = VK_FORMAT_R32G32B32A32_SFLOAT;
	attributeDescriptions[3].offset = offsetof(ft::InstanceData, model) + 48;

	attributeDescriptions[4].binding = 1;
	attributeDescriptions[4].location = 8;
	attributeDescriptions[4].format = VK_FORMAT_R32G32B32A32_SFLOAT;
	attributeDescriptions[4].offset = offsetof(ft::InstanceData, normalMatrix);

	attributeDescriptions[5].binding = 1;
	attributeDescriptions[5].location = 9;
	attributeDescriptions[5].format = VK_FORMAT_R32G32B32A32_SFLOAT;
	attributeDescriptions[5].offset = offsetof(ft::InstanceData, normalMatrix) + 16;

	attributeDescriptions[6].binding = 1;
	attributeDescriptions[6].location = 10;
	attributeDescriptions[6].format = VK_FORMAT_R32G32B32A32_SFLOAT;
	attributeDescriptions[6].offset = offsetof(ft::InstanceData, normalMatrix) + 32;

	attributeDescriptions[7].binding = 1;
	attributeDescriptions[7].location = 11;
	attributeDescriptions[7].format = VK_FORMAT_R32G32B32A32_SFLOAT;
	attributeDescriptions[7].offset = offsetof(ft::InstanceData, normalMatrix) + 48;

	attributeDescriptions[8].binding = 1;
	attributeDescriptions[8].location = 12;
	attributeDescriptions[8].format = VK_FORMAT_R32G32B32_SFLOAT;
	attributeDescriptions[8].offset = offsetof(ft::InstanceData, color);

	attributeDescriptions[9].binding = 1;
	attributeDescriptions[9].location = 13;
	attributeDescriptions[9].format = VK_FORMAT_R32G32B32_SFLOAT;
	attributeDescriptions[9].offset = offsetof(ft::InstanceData, id);

	return attributeDescriptions;
}
