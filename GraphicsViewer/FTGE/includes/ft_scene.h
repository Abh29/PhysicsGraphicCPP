#ifndef FTGRAPHICS_FT_SCENE_H
#define FTGRAPHICS_FT_SCENE_H

#include "ft_headers.h"
#include "ft_defines.h"
#include "ft_model.h"
#include "ft_device.h"
#include "ft_command.h"
#include "ft_buffer.h"
#include "ft_swapChain.h"
#include "ft_camera.h"
#include "ft_pipeline.h"
#include "ft_material.h"
#include "ft_rendering_systems.h"

namespace ft {


	class Scene {

	public:
		using pointer = std::shared_ptr<Scene>;

		Scene(Device::pointer device, std::vector<Buffer::pointer> ubos);
		~Scene() = default;


		void drawSimpleObjs(CommandBuffer::pointer &commandBuffer, const GraphicsPipeline::pointer& pipeline, uint32_t index);
        void drawTexturedObjs(CommandBuffer::pointer, GraphicsPipeline::pointer, TexturedRdrSys::pointer, uint32_t index);
        void drawPickableObjs(const CommandBuffer::pointer &commandBuffer, const GraphicsPipeline::pointer& pipeline, uint32_t index);
		uint32_t addObjectToTheScene(std::string objectPath, InstanceData data);
		uint32_t addObjectCopyToTheScene(uint32_t id, InstanceData data);
        void addMaterialToObject(uint32_t id, Material::pointer material);
		void addPointLightToTheScene(PointLightObject& pl);
		[[nodiscard]] Camera::pointer getCamera() const;
		void setCamera(Camera::pointer camera);
		void setGeneralLight(glm::vec3 color, glm::vec3 direction, float ambient);
		void updateCameraUBO();
		PointLightObject* getLights();
		PushConstantObject& getGeneralLighting();
		[[nodiscard]] std::vector<Model::pointer> getModels() const;
        void setMaterialPool(MaterialPool::pointer pool);

	private:

		Device::pointer												_ftDevice;
		std::vector<Model::pointer>									_models;
		std::vector<Buffer::pointer>								_ftUniformBuffers;
		PushConstantObject											_generalLighting;
		Camera::pointer												_camera;
		UniformBufferObject											_ubo;
        MaterialPool::pointer                                       _ftMaterialPool;
        std::map<uint32_t, std::vector<Model::pointer>>             _materialToModel;
	};

}



#endif //FTGRAPHICS_FT_SCENE_H
