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

	class InstanceRdrSys final : public RenderingSystem {
		
	public:
		using pointer = std::shared_ptr<InstanceRdrSys>;

		explicit InstanceRdrSys(Device::pointer, Renderer::pointer, DescriptorPool::pointer);
		~InstanceRdrSys() override = default;

		void populateUBODescriptors(std::vector<Buffer::pointer> ubos);
        [[nodiscard]] std::vector<DescriptorSet::pointer> getDescriptorSets() const;

	private:
		void createDescriptors();
		void createGraphicsPipeline();

        std::vector<DescriptorSet::pointer>			_ftDescriptorSets;
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

	class OneTextureRdrSys final : public RenderingSystem {
	public:
		using pointer = std::shared_ptr<OneTextureRdrSys>;

		OneTextureRdrSys(Device::pointer, Renderer::pointer, DescriptorPool::pointer);
		~OneTextureRdrSys() override = default;

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

   	class TwoTextureRdrSys final : public RenderingSystem {
	public:
		using pointer = std::shared_ptr<TwoTextureRdrSys>;

        TwoTextureRdrSys(Device::pointer, Renderer::pointer, DescriptorPool::pointer);
		~TwoTextureRdrSys() override = default;

        void populateUBO(const Buffer::pointer& ubo, uint32_t index);
        void populateTextureImage(const Image::pointer& texture, const Sampler::pointer& sampler, uint32_t index);
        void populateNormalImage(const Image::pointer& texture, const Sampler::pointer& sampler, uint32_t index);
        void bindDescriptorSet(const CommandBuffer::pointer& commandBuffer, uint32_t index);

        [[nodiscard]] std::vector<DescriptorSet::pointer> getDescriptorSets() const;
        [[nodiscard]] uint32_t getNormalsImageBinding() const;
        [[nodiscard]] uint32_t getTextureImageBinding() const;
        [[nodiscard]] uint32_t getUboBinding() const;

	private:
		void createDescriptorSetLayout();
		void createGraphicsPipeline();
        void createDescriptors();

        uint32_t                                    _uboBinding;
        uint32_t                                    _textureImageBinding;
        uint32_t                                    _normalsImageBinding;
        std::vector<DescriptorSet::pointer>			_ftDescriptorSets;
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
