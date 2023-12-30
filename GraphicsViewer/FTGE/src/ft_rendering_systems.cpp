#include "../includes/ft_rendering_systems.h"

ft::RenderingSystem::RenderingSystem(Device::pointer device) : _ftDevice(std::move(device)){
	createDescriptorPool();
}

void ft::RenderingSystem::createDescriptorPool() {
	VkDescriptorPoolSize pool_sizes[] =
	{
			{ VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
			{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
			{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
			{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
	};

	VkDescriptorPoolCreateInfo pool_info = {};
	pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
	pool_info.maxSets = 1000;
	pool_info.poolSizeCount = std::size(pool_sizes);
	pool_info.pPoolSizes = pool_sizes;

	if (vkCreateDescriptorPool(_ftDevice->getVKDevice(), &pool_info, nullptr, &_descriptorPool) != VK_SUCCESS) {
		throw std::runtime_error("failed to create a descriptor pool!");
	}
}

ft::GraphicsPipeline::pointer ft::RenderingSystem::getGraphicsPipeline() const {return _ftPipeline;}

VkDescriptorPool ft::RenderingSystem::getVKDescriptorPool() const {return _descriptorPool;}

VkDescriptorSetLayout ft::RenderingSystem::getVKDescriptorSetLayout() const {return _descriptorSetLayout;}

std::vector<VkDescriptorSet> ft::RenderingSystem::getVKDescriptorSets() const {return _descriptorSets;}

/*********************************SimpleRdrSys****************************************/

ft::SimpleRdrSys::SimpleRdrSys(Device::pointer device) : RenderingSystem(std::move(device)){}

ft::SimpleRdrSys::~SimpleRdrSys() {
	vkDestroyDescriptorPool(_ftDevice->getVKDevice(), _descriptorPool, nullptr);
	vkDestroyDescriptorSetLayout(_ftDevice->getVKDevice(), _descriptorSetLayout, nullptr);
}

void ft::SimpleRdrSys::createDescriptorSetLayout() {
// create a descriptor binding for the uniform buffer object layout
	VkDescriptorSetLayoutBinding	uboLayoutBinding{};
	uboLayoutBinding.binding = 0;
	uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	uboLayoutBinding.descriptorCount = 1;
	uboLayoutBinding.stageFlags = VK_SHADER_STAGE_ALL_GRAPHICS;
	uboLayoutBinding.pImmutableSamplers = nullptr;

	// create a descriptor binding for the sampler layout
	VkDescriptorSetLayoutBinding	samplerLayoutBinding{};
	samplerLayoutBinding.binding = 1;
	samplerLayoutBinding.descriptorCount = 1;
	samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	std::vector<VkDescriptorSetLayoutBinding> bindings = {
			uboLayoutBinding,
			samplerLayoutBinding
	};


	VkDescriptorSetLayoutCreateInfo layoutCreateInfo{};
	layoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutCreateInfo.bindingCount = (uint32_t) bindings.size();
	layoutCreateInfo.pBindings = bindings.data();

	if (vkCreateDescriptorSetLayout(_ftDevice->getVKDevice(), &layoutCreateInfo, nullptr, &_descriptorSetLayout) != VK_SUCCESS) {
		throw std::runtime_error("failed to create a descriptor set layout!");
	}
}

void ft::SimpleRdrSys::createDescriptorSets() {
	_descriptorSets.resize(MAX_FRAMES_IN_FLIGHT);

	// allocate the descriptor sets
	std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, _descriptorSetLayout);
	VkDescriptorSetAllocateInfo descriptorSetAllocateInfo{};
	descriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	descriptorSetAllocateInfo.descriptorPool = _descriptorPool;
	descriptorSetAllocateInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
	descriptorSetAllocateInfo.pSetLayouts = layouts.data();

	if (vkAllocateDescriptorSets(_ftDevice->getVKDevice(), &descriptorSetAllocateInfo, _descriptorSets.data()) != VK_SUCCESS) {
		throw std::runtime_error("failed to create the descriptor sets!");
	}
}

void ft::SimpleRdrSys::createGraphicsPipeline() {

}

