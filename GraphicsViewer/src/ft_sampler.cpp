#include "../includes/ft_sampler.h"

ft::Sampler::Sampler(Device::pointer device) : _ftDevice(std::move(device)) {
	VkPhysicalDeviceProperties properties = _ftDevice->getVKPhysicalDeviceProperties();

	VkSamplerCreateInfo samplerCreateInfo{};
	samplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerCreateInfo.magFilter = VK_FILTER_LINEAR;
	samplerCreateInfo.minFilter = VK_FILTER_LINEAR;
	samplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerCreateInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerCreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerCreateInfo.anisotropyEnable = VK_TRUE;
	samplerCreateInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
	samplerCreateInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	samplerCreateInfo.unnormalizedCoordinates = VK_FALSE;
	samplerCreateInfo.compareEnable = VK_FALSE;
	samplerCreateInfo.compareOp = VK_COMPARE_OP_ALWAYS;
	samplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerCreateInfo.mipLodBias = 0.0f;
	samplerCreateInfo.minLod = 0.0f;
	samplerCreateInfo.maxLod = VK_LOD_CLAMP_NONE;

	if (vkCreateSampler(_ftDevice->getVKDevice(), &samplerCreateInfo, nullptr, &_sampler) != VK_SUCCESS) {
		throw std::runtime_error("failed to create a texture sampler!");
	}
}

ft::Sampler::Sampler(Device::pointer device, VkSamplerCreateInfo &createInfo)
: _ftDevice(device) {
	if (vkCreateSampler(_ftDevice->getVKDevice(), &createInfo, nullptr, &_sampler) != VK_SUCCESS) {
		throw std::runtime_error("failed to create a texture sampler!");
	}
}

ft::Sampler::~Sampler() {
	vkDestroySampler(_ftDevice->getVKDevice(), _sampler, nullptr);
}

VkSampler ft::Sampler::getVKSampler() const {return _sampler;}