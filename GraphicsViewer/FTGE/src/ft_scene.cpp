#include "../include.h"
#include "../includes/ft_scene.h"


ft::Scene::Scene(ft::Device::pointer device, ft::CommandPool::pointer pool, std::vector<ft::Buffer::pointer> ubos):
_ftDevice(device), _ftCommandPool(pool), _ftUniformBuffers(ubos) {

	// point lights
	_ubo.pLCount = 0;

	// general lighting
	_generalLighting.lightColor = {1.0f, 1.0f, 1.0f};
	_generalLighting.lightDirection = {10.0f, 10.0f, 10.0f};
	_generalLighting.ambient = 0.2f;
}

void ft::Scene::drawScene(ft::CommandBuffer::pointer commandBuffer, VkPipelineLayout layout, uint32_t index) {

	// push constant
	vkCmdPushConstants(commandBuffer->getVKCommandBuffer(), layout, VK_SHADER_STAGE_VERTEX_BIT,
					   0, static_cast<uint32_t>(sizeof(_generalLighting)), &_generalLighting);

	_ftUniformBuffers[index]->copyToMappedData(&_ubo, sizeof(_ubo));
	// vertex and index buffers
	for (auto& model : _models) {
		model->bind(commandBuffer, index);
		model->draw(commandBuffer);
	}
}

uint32_t ft::Scene::addObjectToTheScene(std::string objectPath, ft::InstanceData data) {
	Model::pointer model = std::make_shared<Model>(_ftDevice, _ftCommandPool, objectPath, _ftUniformBuffers.size());
	data.normalMatrix = glm::inverseTranspose(data.model * _ubo.view);
	model->getCopies()[0] = data;
	_models.push_back(model);
	return model->getFirstId();
}

void ft::Scene::addPointLightToTheScene(PointLightObject &pl) {
	_ubo.lights[_ubo.pLCount++] = pl;
}

uint32_t ft::Scene::addObjectCopyToTheScene(uint32_t id, ft::InstanceData data) {
	data.normalMatrix = glm::inverseTranspose(data.model * _ubo.view);
	for (auto& m : _models) {
		if (m->findID(id))
			return m->addCopy(data);
	}
	return -1;
}

ft::Camera::pointer ft::Scene::getCamera() const {
	return _camera;
}

void ft::Scene::setCamera(ft::Camera::pointer camera) {
	_camera = std::move(camera);
	_ubo.view = _camera->getViewMatrix();
	_ubo.proj = _camera->getProjMatrix();
}

void ft::Scene::setGeneralLight(glm::vec3 color, glm::vec3 direction, float ambient) {
	_generalLighting.lightColor = color;
	_generalLighting.lightDirection = direction;
	_generalLighting.ambient = ambient;
}

void ft::Scene::updateCameraUBO() {
	_ubo.view = _camera->getViewMatrix();
	_ubo.proj = _camera->getProjMatrix();
}

ft::PushConstantObject &ft::Scene::getGeneralLighting() {return _generalLighting;}

ft::PointLightObject* ft::Scene::getLights() {return _ubo.lights;}

std::vector<ft::Model::pointer> ft::Scene::getModels() const {return _models;}