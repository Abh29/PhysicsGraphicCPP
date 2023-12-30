#include "../includes/ft_descriptor.h"

ft::DescriptorSetLayout::DescriptorSetLayout(Device::pointer device,
											 std::vector<VkDescriptorSetLayoutBinding> bindings):
											 _ftDevice(std::move(device))
											 {
												 std::for_each(bindings.begin(), bindings.end(),
															   [&](VkDescriptorSetLayoutBinding& b) {
													 if (_descriptorBindings.count(b.binding))
														 throw std::runtime_error("binding already used!");
													 _descriptorBindings[b.binding] = b;
												 });

												 VkDescriptorSetLayoutCreateInfo layoutCreateInfo{};
												 layoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
												 layoutCreateInfo.bindingCount = (uint32_t) bindings.size();
												 layoutCreateInfo.pBindings = bindings.data();

												 if (vkCreateDescriptorSetLayout(_ftDevice->getVKDevice(), &layoutCreateInfo, nullptr, &_descriptorLayout) != VK_SUCCESS) {
													 throw std::runtime_error("failed to create a descriptor set layout!");
												 }
											 }

ft::DescriptorSetLayout::~DescriptorSetLayout() {
	vkDestroyDescriptorSetLayout(_ftDevice->getVKDevice(), _descriptorLayout, nullptr);
}

std::vector<VkDescriptorSetLayoutBinding> ft::DescriptorSetLayout::getVKBindings() const {
	std::vector<VkDescriptorSetLayoutBinding> v(_descriptorBindings.size());

	std::transform(_descriptorBindings.begin(), _descriptorBindings.end(), v.begin(),
				   [](auto &i) {
		return i.second;
	});
	return v;
}

VkDescriptorSetLayout& ft::DescriptorSetLayout::getVKLayout() {return _descriptorLayout;}

/*********************************DescriptorSetLayoutBuilder********************************/

ft::DescriptorSetLayoutBuilder &
ft::DescriptorSetLayoutBuilder::addDescriptorBinding(VkDescriptorType type, VkShaderStageFlags flags, int32_t count,
													 const VkSampler *imSampler) {
	VkDescriptorSetLayoutBinding	binding{};
	binding.binding = _descriptorBindings.size();
	binding.descriptorType = type;
	binding.descriptorCount = count;
	binding.stageFlags = flags;
	binding.pImmutableSamplers = imSampler;
	_descriptorBindings.push_back(binding);
	return *this;
}

ft::DescriptorSetLayout::pointer ft::DescriptorSetLayoutBuilder::build(Device::pointer device) {
	return std::make_shared<DescriptorSetLayout>(std::move(device), _descriptorBindings);
}

void ft::DescriptorSetLayoutBuilder::reset() {_descriptorBindings.clear();}


/********************************DescriptorSet************************************************/

ft::DescriptorSet::DescriptorSet(Device::pointer device, DescriptorSetLayout::pointer layout,
								 DescriptorPool& pool):
								 _ftDevice(std::move(device)),
								 _ftDSLayout(std::move(layout)) {

	VkDescriptorSetAllocateInfo descriptorSetAllocateInfo{};
	descriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	descriptorSetAllocateInfo.descriptorPool = pool.getVKAvailablePool();
	descriptorSetAllocateInfo.descriptorSetCount = 1;
	descriptorSetAllocateInfo.pSetLayouts = &_ftDSLayout->getVKLayout();

	if (vkAllocateDescriptorSets(_ftDevice->getVKDevice(), &descriptorSetAllocateInfo, &_descriptorSet) != VK_SUCCESS) {
		throw std::runtime_error("failed to create the descriptor sets!");
	}
}

VkDescriptorSet ft::DescriptorSet::getVKDescriptorSet() const {return _descriptorSet;}

bool ft::DescriptorSet::updateDescriptorBuffer(uint32_t binding, VkDescriptorType type, const Buffer::pointer& buffer,
											   size_t offset) {

	VkDescriptorBufferInfo descriptorBufferInfo{};
	descriptorBufferInfo.buffer = buffer->getVKBuffer();
	descriptorBufferInfo.offset = offset;
	descriptorBufferInfo.range = buffer->getSize();

	VkWriteDescriptorSet writeDescriptorSet{};
	writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	writeDescriptorSet.dstSet = _descriptorSet;
	writeDescriptorSet.dstBinding = binding;
	writeDescriptorSet.dstArrayElement = 0;
	writeDescriptorSet.descriptorType = type;
	writeDescriptorSet.descriptorCount = 1;
	writeDescriptorSet.pBufferInfo = &descriptorBufferInfo;
	writeDescriptorSet.pImageInfo = nullptr;
	writeDescriptorSet.pTexelBufferView = nullptr;

	vkUpdateDescriptorSets(_ftDevice->getVKDevice(), 1, &writeDescriptorSet, 0, nullptr);
	return true;
}

bool ft::DescriptorSet::updateDescriptorImage(uint32_t binding, VkDescriptorType type, VkImageLayout layout,
											  const Image::pointer& image, const Sampler::pointer& sampler) {

	VkDescriptorImageInfo descriptorImageInfo{};
	descriptorImageInfo.imageLayout = layout;
	descriptorImageInfo.imageView = image->getVKImageView();
	descriptorImageInfo.sampler = sampler->getVKSampler();

	VkWriteDescriptorSet writeDescriptorSet{};
	writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	writeDescriptorSet.dstSet = _descriptorSet;
	writeDescriptorSet.dstBinding = binding;
	writeDescriptorSet.dstArrayElement = 0;
	writeDescriptorSet.descriptorType = type;
	writeDescriptorSet.descriptorCount = 1;
	writeDescriptorSet.pBufferInfo = nullptr;
	writeDescriptorSet.pImageInfo = &descriptorImageInfo;
	writeDescriptorSet.pTexelBufferView = nullptr;

	vkUpdateDescriptorSets(_ftDevice->getVKDevice(), 1, &writeDescriptorSet, 0, nullptr);
	return true;
}


/***********************************DescriptorPool****************************************/

ft::DescriptorPool::DescriptorPool(Device::pointer device) : _ftDevice(std::move(device)), _currentPool(0) {
	VkDescriptorPool pool;
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

	if (vkCreateDescriptorPool(_ftDevice->getVKDevice(), &pool_info, nullptr, &pool) != VK_SUCCESS) {
		throw std::runtime_error("failed to create a descriptor pool!");
	}
}

ft::DescriptorPool::~DescriptorPool() {
	for(auto& p : _descriptorsPools) {
		vkDestroyDescriptorPool(_ftDevice->getVKDevice(), p, nullptr);
	}
}

VkDescriptorPool ft::DescriptorPool::getVKAvailablePool() const {return _descriptorsPools[_currentPool];}

void ft::DescriptorPool::resetPool() {
	for(auto& p : _descriptorsPools) {
		vkResetDescriptorPool(_ftDevice->getVKDevice(), p, VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT);
	}
	_currentPool = 0;
}

void ft::DescriptorPool::createNewPool() {
	VkDescriptorPool pool;
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

	if (vkCreateDescriptorPool(_ftDevice->getVKDevice(), &pool_info, nullptr, &pool) != VK_SUCCESS) {
		throw std::runtime_error("failed to create a descriptor pool!");
	}
	++_currentPool;
}

ft::DescriptorSet::pointer ft::DescriptorPool::allocateSet(DescriptorSetLayout::pointer layout) {
	try {
		auto p = std::make_shared<DescriptorSet>(_ftDevice, layout, *this);
		return p;
	} catch (std::exception& e) {
		createNewPool();
		auto p = std::make_shared<DescriptorSet>(_ftDevice, layout, *this);
		return p;
	}
}

