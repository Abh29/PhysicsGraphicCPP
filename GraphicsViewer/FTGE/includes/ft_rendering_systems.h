#ifndef FTGRAPHICS_FT_RENDERING_SYSTEMS_H
#define FTGRAPHICS_FT_RENDERING_SYSTEMS_H

#include "ft_headers.h"
#include "ft_defines.h"
#include "ft_device.h"
#include "ft_pipeline.h"


namespace ft {

	class RenderingSystem {

	public:
		using pointer = std::shared_ptr<RenderingSystem>;

		explicit RenderingSystem(Device::pointer);
		virtual ~RenderingSystem() = default;

		void createDescriptorPool();

		[[nodiscard]] GraphicsPipeline::pointer getGraphicsPipeline() const;
		[[nodiscard]] VkDescriptorSetLayout getVKDescriptorSetLayout() const;
		[[nodiscard]] std::vector<VkDescriptorSet> getVKDescriptorSets() const;
		[[nodiscard]] VkDescriptorPool getVKDescriptorPool() const;

	protected:

		Device::pointer 							_ftDevice;
		GraphicsPipeline::pointer					_ftPipeline;
		VkDescriptorSetLayout 						_descriptorSetLayout;
		VkDescriptorPool 							_descriptorPool;
		std::vector<VkDescriptorSet>				_descriptorSets;

	};


	class SimpleRdrSys : public RenderingSystem {
		
	public:
		using pointer = std::shared_ptr<SimpleRdrSys>;

		explicit SimpleRdrSys(Device::pointer);
		~SimpleRdrSys() override;

		void createDescriptorSetLayout();
		void createDescriptorSets();
		void createGraphicsPipeline();

	private:
		Image::pointer							_ftDepthImage;
		Image::pointer							_ftColorImage;

	};




}

#endif //FTGRAPHICS_FT_RENDERING_SYSTEMS_H
