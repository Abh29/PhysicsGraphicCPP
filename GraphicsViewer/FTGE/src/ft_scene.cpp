#include "../includes/ft_scene.h"


ft::Scene::Scene(ft::Device::pointer device, std::vector<ft::Buffer::pointer> ubos):
_ftDevice(device), _ftUniformBuffers(ubos) {

	// point lights
	_ubo.pLCount = 0;

	// general lighting
	_generalLighting.lightColor = {1.0f, 1.0f, 1.0f};
	_generalLighting.lightDirection = {10.0f, 10.0f, 10.0f};
	_generalLighting.ambient = 0.2f;
}

void ft::Scene::drawSimpleObjs(CommandBuffer::pointer &commandBuffer, const GraphicsPipeline::pointer& pipeline, uint32_t index) {

	// push constant
	vkCmdPushConstants(commandBuffer->getVKCommandBuffer(), pipeline->getVKPipelineLayout(), VK_SHADER_STAGE_VERTEX_BIT,
					   0, static_cast<uint32_t>(sizeof(_generalLighting)), &_generalLighting);

	_ftUniformBuffers[index]->copyToMappedData(&_ubo, sizeof(_ubo));
	// vertex and index buffers
	for (auto& model : _models) {
        if (model->hasMaterial()) continue;
		model->bind(commandBuffer, index);
		model->draw(commandBuffer);
	}
}

void ft::Scene::drawTexturedObjs(ft::CommandBuffer::pointer commandBuffer, ft::GraphicsPipeline::pointer pipeline, ft::TexturedRdrSys::pointer system,
                                 uint32_t index) {

    // bind the graphics pipeline
    vkCmdBindPipeline(commandBuffer->getVKCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, system->getGraphicsPipeline()->getVKPipeline());

    // push constant
    vkCmdPushConstants(commandBuffer->getVKCommandBuffer(), pipeline->getVKPipelineLayout(), VK_SHADER_STAGE_VERTEX_BIT,
                       0, static_cast<uint32_t>(sizeof(_generalLighting)), &_generalLighting);

    _ftUniformBuffers[index]->copyToMappedData(&_ubo, sizeof(_ubo));

    // vertex and index buffers
    for (auto & i : _materialToModel) {
        auto mat = _ftMaterialPool->getMaterialByID(i.first);
        system->populateUBODescriptors(_ftUniformBuffers, mat);
        system->populateTextureDescriptors(mat);
//        mat->getDescriptorSet(index)->updateDescriptorBuffer()
        // bind the descriptor sets
        vkCmdBindDescriptorSets(commandBuffer->getVKCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS,
                                system->getGraphicsPipeline()->getVKPipelineLayout(), 0, 1,
                                &(mat->getDescriptorSet(index)->getVKDescriptorSet()),
                                0, nullptr);
        for (auto& model : i.second) {
            model->bind(commandBuffer, index);
            model->draw(commandBuffer);
        }
    }
}

uint32_t ft::Scene::addObjectToTheScene(std::string objectPath, ft::InstanceData data) {
	Model::pointer model = std::make_shared<Model>(_ftDevice, objectPath, _ftUniformBuffers.size());
	data.normalMatrix = glm::inverseTranspose(data.model * _ubo.view);
//	data.id = Model::uint32ToVec3(model->getFirstId());
	data.id = model->getFirstId();
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

void ft::Scene::addMaterialToObject(uint32_t id, Material::pointer material) {
    for (auto& m : _models) {
        if (m->findID(id)) {
            m->setMaterial(material);
            if (_materialToModel.find(material->getID()) == _materialToModel.end()) {
                _materialToModel[material->getID()] = {m};
            } else {
                _materialToModel[material->getID()].push_back(m);
            }
            break;
        }
    }
}

void ft::Scene::setMaterialPool(MaterialPool::pointer pool) {_ftMaterialPool = std::move(pool);}

void
ft::Scene::drawPickableObjs(const ft::CommandBuffer::pointer &commandBuffer, const ft::GraphicsPipeline::pointer &pipeline,
                            uint32_t index) {
    (void) pipeline;
    // vertex and index buffers
    for (auto& model : _models) {
        model->bind(commandBuffer, index);
        model->draw(commandBuffer);
    }
}

