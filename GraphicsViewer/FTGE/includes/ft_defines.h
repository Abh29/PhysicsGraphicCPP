#ifndef FTGRAPHICS_FT_DEFINES_H
#define FTGRAPHICS_FT_DEFINES_H

#include "ft_headers.h"

namespace ft {

	struct QueueFamilyIndices {
		std::optional<uint32_t> graphicsFamily;
		std::optional<uint32_t> presentFamily;

		[[nodiscard]] bool isComplete() const;
	};

	struct SwapChainSupportDetails {
		VkSurfaceCapabilitiesKHR capabilities;
		std::vector<VkSurfaceFormatKHR> formats;
		std::vector<VkPresentModeKHR> presentModes;
	};

	struct UniformBufferObject {
		alignas(16) glm::mat4	model;
		alignas(16) glm::mat4 	view;
		alignas(16) glm::mat4 	proj;
	};

//	static std::vector<char> readFile(const std::string& filename) {
//		std::ifstream file(filename, std::ios::ate | std::ios::binary);
//		if (!file.is_open()) {
//			throw std::runtime_error("failed to open file " + filename);
//		}
//		std::streamsize fileSize = file.tellg();
//		std::vector<char> buffer(fileSize);
//		file.seekg(0);
//		file.read(buffer.data(), fileSize);
//		file.close();
//		return buffer;
//	}



}

#endif //FTGRAPHICS_FT_DEFINES_H
