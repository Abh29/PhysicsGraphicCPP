#ifndef FTGRAPHICS_FT_RENDERING_SYSTEMS_H
#define FTGRAPHICS_FT_RENDERING_SYSTEMS_H

#include "ft_headers.h"
#include "ft_defines.h"
#include "ft_device.h"
#include "ft_pipeline.h"
#include "ft_descriptor.h"
#include "ft_renderer.h"
#include "ft_vertex.h"
#include "ft_texture.h"


namespace ft {

	class RenderingSystem {

	public:
		using pointer = std::shared_ptr<RenderingSystem>;

		RenderingSystem(Device::pointer, Renderer::pointer, DescriptorPool::pointer);
		virtual ~RenderingSystem() = default;


		[[nodiscard]] GraphicsPipeline::pointer getGraphicsPipeline() const;
		[[nodiscard]] DescriptorSetLayout::pointer getDescriptorSetLayout() const;
		[[nodiscard]] DescriptorPool::pointer getDescriptorPool() const;

	protected:

		Device::pointer 							_ftDevice;
		Renderer::pointer							_ftRenderer;
		DescriptorPool::pointer						_ftDescriptorPool;
		DescriptorSetLayout::pointer 				_ftDescriptorSetLayout;
		GraphicsPipeline::pointer					_ftPipeline;

	};


	class SimpleRdrSys final : public RenderingSystem {
		
	public:
		using pointer = std::shared_ptr<SimpleRdrSys>;

		explicit SimpleRdrSys(Device::pointer, Renderer::pointer, DescriptorPool::pointer);
		~SimpleRdrSys() override = default;

		void populateUBODescriptors(std::vector<Buffer::pointer> ubos);
        [[nodiscard]] std::vector<DescriptorSet::pointer> getDescriptorSets() const;

	private:
		void createDescriptors();
		void createGraphicsPipeline();

        std::vector<DescriptorSet::pointer>			_ftDescriptorSets;

	};

	class TexturedRdrSys final : public RenderingSystem {
	public:
		using pointer = std::shared_ptr<TexturedRdrSys>;

		TexturedRdrSys(Device::pointer, Renderer::pointer, DescriptorPool::pointer);
		~TexturedRdrSys() override = default;

		void populateUBODescriptors(std::vector<Buffer::pointer> ubos, const Texture::pointer& material);
		void populateTextureDescriptors(const Texture::pointer& material);
        [[nodiscard]] uint32_t getTextureImageBinding() const;
        [[nodiscard]] uint32_t getSamplerBinding() const;

	private:
		void createDescriptorSetLayout();
		void createGraphicsPipeline();

        uint32_t                        _samplerBinding;
        uint32_t                        _textureImageBinding;
	};

    class PickingRdrSys final : public RenderingSystem {
    public:
        using pointer = std::shared_ptr<PickingRdrSys>;

        PickingRdrSys(Device::pointer, Renderer::pointer, DescriptorPool::pointer);
        ~PickingRdrSys() override = default;

        void populateUBODescriptors(std::vector<Buffer::pointer> ubos);
        [[nodiscard]] std::vector<DescriptorSet::pointer> getDescriptorSets() const;
        void createGraphicsPipeline(const RenderPass::pointer& renderPass);

    private:
        void createDescriptorSetLayout();

        std::vector<DescriptorSet::pointer>                     _ftDescriptorSets;
        bool                                                    _isCreatedPipeline = false;

    };

}

#endif //FTGRAPHICS_FT_RENDERING_SYSTEMS_H
