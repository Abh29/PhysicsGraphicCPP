#ifndef FTGRAPHICS_FT_SAMPLER_H
#define FTGRAPHICS_FT_SAMPLER_H

#include "ft_headers.h"
#include "ft_device.h"

namespace ft {

	class Sampler {

	public:
		using pointer = std::shared_ptr<Sampler>;
		Sampler(Device::pointer device);
		Sampler(Device::pointer device, VkSamplerCreateInfo &createInfo);
		~Sampler();

		[[nodiscard]] VkSampler getVKSampler() const;

	private:
		Device::pointer 			_ftDevice;
		VkSampler 					_sampler = VK_NULL_HANDLE;

	};

}



#endif //FTGRAPHICS_FT_SAMPLER_H
