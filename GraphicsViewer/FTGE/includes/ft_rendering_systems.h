#ifndef FTGRAPHICS_FT_RENDERING_SYSTEMS_H
#define FTGRAPHICS_FT_RENDERING_SYSTEMS_H

#include "ft_headers.h"
#include "ft_defines.h"
#include "ft_device.h"
#include "ft_pipeline.h"
#include "ft_descriptor.h"
#include "ft_renderer.h"
#include "ft_vertex.h"


namespace ft {

	class RenderingSystem {

	public:
		using pointer = std::shared_ptr<RenderingSystem>;

		RenderingSystem(Device::pointer, Renderer::pointer, DescriptorPool::pointer);
		virtual ~RenderingSystem() = default;


		[[nodiscard]] GraphicsPipeline::pointer getGraphicsPipeline() const;
		[[nodiscard]] DescriptorSetLayout::pointer getDescriptorSetLayout() const;
		[[nodiscard]] std::vector<DescriptorSet::pointer> getDescriptorSets() const;
		[[nodiscard]] DescriptorPool::pointer getDescriptorPool() const;

	protected:

		Device::pointer 							_ftDevice;
		Renderer::pointer							_ftRenderer;
		DescriptorPool::pointer						_ftDescriptorPool;
		DescriptorSetLayout::pointer 				_ftDescriptorSetLayout;
		std::vector<DescriptorSet::pointer>			_ftDescriptorSets;
		GraphicsPipeline::pointer					_ftPipeline;

	};


	class SimpleRdrSys final : public RenderingSystem {
		
	public:
		using pointer = std::shared_ptr<SimpleRdrSys>;

		explicit SimpleRdrSys(Device::pointer, Renderer::pointer, DescriptorPool::pointer);
		~SimpleRdrSys() override = default;

		void populateUBODescriptors(std::vector<Buffer::pointer> ubos);

	private:
		void createDescriptors();
		void createGraphicsPipeline();

	};

	class TexturedRdrSys final : public RenderingSystem {

	};


}

#endif //FTGRAPHICS_FT_RENDERING_SYSTEMS_H
