#ifndef PLAYGROUND_VERTEX_H
#define PLAYGROUND_VERTEX_H

#include "ft_headers.h"

struct Vertex {
	glm::vec3	pos;
	glm::vec3 	color;
	glm::vec2 	texCoord;
	glm::vec3 	normal;

	Vertex(glm::vec3 p = {}, glm::vec3 c = {0.1f, 1.0f, 0.5f}, glm::vec2 t = {});

	static VkVertexInputBindingDescription getBindingDescription();

	static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescription();

	bool operator==(const Vertex& other) const;
};


namespace std {
	template<> struct hash<Vertex> {
		size_t operator()(const Vertex& vertex) const;
	};
}


#endif //PLAYGROUND_VERTEX_H
