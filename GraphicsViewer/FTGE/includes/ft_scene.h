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

namespace ft {


	class Scene {

	public:
		using pointer = std::shared_ptr<Scene>;

		Scene(Device::pointer device, CommandPool::pointer pool, std::vector<Buffer::pointer> ubos);
		~Scene() = default;

		void drawScene(CommandBuffer::pointer commandBuffer, VkPipelineLayout layout, uint32_t index);
		uint32_t addObjectToTheScene(std::string objectPath, InstanceData data);
		uint32_t addObjectCopyToTheScene(uint32_t id, InstanceData data);
		void addPointLightToTheScene(PointLightObject& pl);
		[[nodiscard]] Camera::pointer getCamera() const;
		void setCamera(Camera::pointer camera);
		void setGeneralLight(glm::vec3 color, glm::vec3 direction, float ambient);
		void updateCameraUBO();
		PointLightObject* getLights();
		PushConstantObject& getGeneralLighting();
		[[nodiscard]] std::vector<Model::pointer> getModels() const;

	private:

		Device::pointer												_ftDevice;
		CommandPool::pointer										_ftCommandPool;
		std::vector<Model::pointer>									_models;
		std::vector<Buffer::pointer>								_ftUniformBuffers;
		PushConstantObject											_generalLighting;
		Camera::pointer												_camera;
		UniformBufferObject											_ubo;

	};

}



#endif //FTGRAPHICS_FT_SCENE_H
