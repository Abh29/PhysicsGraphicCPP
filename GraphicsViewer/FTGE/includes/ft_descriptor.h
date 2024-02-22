#ifndef FTGRAPHICS_FT_DESCRIPTOR_H
#define FTGRAPHICS_FT_DESCRIPTOR_H

#include "ft_buffer.h"
#include "ft_defines.h"
#include "ft_device.h"
#include "ft_headers.h"
#include "ft_image.h"
#include "ft_sampler.h"

namespace ft {

class DescriptorSet;
class DescriptorSetLayout;

class DescriptorSetLayout {

public:
  using pointer = std::shared_ptr<DescriptorSetLayout>;
  DescriptorSetLayout(Device::pointer device,
                      std::vector<VkDescriptorSetLayoutBinding> bindings);
  ~DescriptorSetLayout();

  [[nodiscard]] std::vector<VkDescriptorSetLayoutBinding> getVKBindings() const;
  [[nodiscard]] VkDescriptorSetLayout &getVKLayout();

private:
  Device::pointer _ftDevice;
  std::map<uint32_t, VkDescriptorSetLayoutBinding> _descriptorBindings;
  VkDescriptorSetLayout _descriptorLayout;
};

class DescriptorSetLayoutBuilder {

public:
  using pointer = std::shared_ptr<DescriptorSetLayoutBuilder>;
  DescriptorSetLayoutBuilder() = default;
  ~DescriptorSetLayoutBuilder() = default;

  DescriptorSetLayoutBuilder &
  addDescriptorBinding(VkDescriptorType type, VkShaderStageFlags flags,
                       int32_t count = 1,
                       const VkSampler *imSampler = VK_NULL_HANDLE);

  DescriptorSetLayout::pointer build(Device::pointer device);
  void reset();

private:
  std::vector<VkDescriptorSetLayoutBinding> _descriptorBindings;
};

class DescriptorPool;

class DescriptorSet {
public:
  using pointer = std::shared_ptr<DescriptorSet>;
  friend class DescriptorPool;

  DescriptorSet(Device::pointer device, DescriptorSetLayout::pointer layout,
                DescriptorPool &pool);
  ~DescriptorSet() = default;

  bool updateDescriptorBuffer(uint32_t binding, VkDescriptorType type,
                              const Buffer::pointer &buffer, size_t offset);
  bool updateDescriptorImage(uint32_t binding, VkDescriptorType type,
                             VkImageLayout layout, const Image::pointer &image,
                             const Sampler::pointer &sampler);
  [[nodiscard]] VkDescriptorSet getVKDescriptorSet() const;
  [[nodiscard]] VkDescriptorSet &getVKDescriptorSet();
  [[nodiscard]] DescriptorSetLayout::pointer getDescriptorSetLayout() const;

private:
  Device::pointer _ftDevice;
  DescriptorSetLayout::pointer _ftDSLayout;
  VkDescriptorSet _descriptorSet{};
};

class DescriptorPool {

public:
  using pointer = std::shared_ptr<DescriptorPool>;

  explicit DescriptorPool(Device::pointer device);
  ~DescriptorPool();

  [[nodiscard]] VkDescriptorPool getVKAvailablePool() const;
  [[nodiscard]] DescriptorSet::pointer
  allocateSet(DescriptorSetLayout::pointer layout);
  void resetPool();

private:
  void createNewPool();

  Device::pointer _ftDevice;
  std::vector<VkDescriptorPool> _descriptorsPools;
  uint32_t _currentPool;
};

} // namespace ft

#endif // FTGRAPHICS_FT_DESCRIPTOR_H
