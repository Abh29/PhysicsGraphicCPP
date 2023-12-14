#ifndef PLAYGROUND_VERTEX_H
#define PLAYGROUND_VERTEX_H

#include "ft_headers.h"

namespace ft {

	struct Vertex {
		alignas(8) glm::vec3	pos;
		alignas(8) glm::vec3 	color;
		alignas(8) glm::vec3 	normal;
		alignas(8) glm::vec2 	texCoord;

		Vertex(glm::vec3 p = {}, glm::vec3 c = {0.1f, 1.0f, 0.5f}, glm::vec3 n = {}, glm::vec2 t = {});

		static VkVertexInputBindingDescription getBindingDescription();
		static std::array<VkVertexInputAttributeDescription, 4> getAttributeDescription();
		bool operator==(const Vertex& other) const;
	};


	struct InstanceData {
		alignas(16) glm::mat4 model;
		alignas(16) glm::vec3 color;

		static VkVertexInputBindingDescription getBindingDescription();
		static std::array<VkVertexInputAttributeDescription, 5> getAttributeDescription();
	};

}



namespace std {
	template<> struct hash<ft::Vertex> {
		size_t operator()(const ft::Vertex& vertex) const;
	};
}


#endif //PLAYGROUND_VERTEX_H
