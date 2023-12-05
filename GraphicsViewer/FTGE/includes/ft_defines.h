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


}

#endif //FTGRAPHICS_FT_DEFINES_H
