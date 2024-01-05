#include "../includes/ft_app.h"

ft::Application::Application() :
_ftEventListener(std::make_shared<ft::EventListener>()),
_ftWindow{std::make_shared<Window>(W_WIDTH, W_HEIGHT, "applicationWindow", nullptr, _ftEventListener)}
{
	_validationLayers = {
			"VK_LAYER_KHRONOS_validation",
	};

	_deviceExtensions = {
			VK_KHR_SWAPCHAIN_EXTENSION_NAME,
	};

	initEventListener();
	initApplication();
	createScene();
}

void ft::Application::run() {

	while(!_ftWindow->shouldClose()) {
		_ftWindow->pollEvents();
		_ftGui->newFrame();
		_ftGui->showDemo();
		drawFrame();
		#ifdef SHOW_FRAME_RATE
			printFPS();
		#endif
	}
	vkDeviceWaitIdle(_ftDevice->getVKDevice());
}

void ft::Application::initEventListener() {
	_ftEventListener->addCallbackForEventType(Event::EventType::KEYBOARD_EVENT, [&](ft::Event& ev) {
		auto& kev = dynamic_cast<KeyboardEvent&>(ev);
		auto data = kev.getData();
		if (std::any_cast<int>(data[2]) == _ftWindow->ACTION(KeyActions::KEY_PRESS) ||
			std::any_cast<int>(data[2]) == _ftWindow->ACTION(KeyActions::KEY_REPEAT))
			updateScene(std::any_cast<int>(data[0]));
	});

	_ftEventListener->addCallbackForEventType(Event::EventType::MOUSE_BUTTON, [&](ft::Event& ev) {
		auto& cev = dynamic_cast<CursorEvent&>(ev);
		auto data = cev.getData();
		if (std::any_cast<int>(data[1]) == _ftWindow->ACTION(KeyActions::KEY_PRESS)) {
			auto x = std::any_cast<double>(data[3]);
			auto y = std::any_cast<double>(data[4]);
			std::cout << "mouse clicked: " << x << " , " << y << std::endl;
		}
	});

	_ftEventListener->addCallbackForEventType(Event::EventType::MOUSE_SCROLL, [&](ft::Event& ev) {
		auto& sev = dynamic_cast<ScrollEvent&>(ev);
		auto data = sev.getData();
		auto yOff = std::any_cast<double>(data[1]);

		_ftScene->getCamera()->forward((float)yOff);
		_ftScene->updateCameraUBO();

	});

    _ftEventListener->addCallbackForEventType(Event::EventType::SCREEN_RESIZE_EVENT, [&](ft::Event& ev) {
        auto& srev = dynamic_cast<ScreenResizeEvent&>(ev);
        auto data = srev.getData();
        auto width = std::any_cast<int>(data[0]);
        auto height = std::any_cast<int>(data[1]);
        _ftScene->getCamera()->updateAspect((float)width / (float)height);
        _ftScene->updateCameraUBO();
    });
}

void ft::Application::initApplication() {

	VkApplicationInfo	applicationInfo{};
	applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	applicationInfo.pApplicationName = "Simple Application";
	applicationInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	applicationInfo.pEngineName = "No Engine";
	applicationInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	applicationInfo.apiVersion = VK_MAKE_API_VERSION(0, 1, 3, 0);
	applicationInfo.pNext = nullptr;

	_ftInstance = std::make_shared<Instance>(applicationInfo, _validationLayers, _ftWindow->getRequiredExtensions());
	_ftSurface = std::make_shared<Surface>(_ftInstance, _ftWindow);
	_ftPhysicalDevice = std::make_shared<PhysicalDevice>(_ftInstance, _ftSurface, _deviceExtensions);
	_ftDevice = std::make_shared<Device>(_ftPhysicalDevice, _validationLayers, _deviceExtensions);
	_ftRenderer = std::make_shared<ft::Renderer>(_ftWindow, _ftSurface, _ftPhysicalDevice, _ftDevice);


	_ftGui = std::make_shared<Gui>(_ftInstance, _ftPhysicalDevice, _ftDevice,
								   _ftWindow, _ftRenderer->getRenderPass(), MAX_FRAMES_IN_FLIGHT);

    _ftMaterialPool = std::make_shared<ft::MaterialPool>(_ftDevice);
	_ftDescriptorPool = std::make_shared<ft::DescriptorPool>(_ftDevice);

	_ftSimpleRdrSys = std::make_shared<ft::SimpleRdrSys>(_ftDevice, _ftRenderer, _ftDescriptorPool);
	_ftSimpleRdrSys->populateUBODescriptors(_ftRenderer->getUniformBuffers());

	_ftTexturedRdrSys = std::make_shared<ft::TexturedRdrSys>(_ftDevice, _ftRenderer, _ftDescriptorPool);

}

//TODO: replace this with a scene manager, read scene from disk
void ft::Application::createScene() {
	CameraBuilder cameraBuilder;
	_ftScene = std::make_shared<Scene>(_ftDevice, _ftRenderer->getUniformBuffers());
    _ftScene->setMaterialPool(_ftMaterialPool);
	_ftScene->setCamera(cameraBuilder.setEyePosition({5,-1,0})
				.setTarget({1,-1,0})
				.setUpDirection({0,1,0})
				.setFOV(120)
				.setZNearFar(0.5f, 1000.0f)
				.setAspect(_ftRenderer->getSwapChain()->getAspect())
				.build());
	_ftScene->setGeneralLight({1.0f,1.0f,1.0f}, {10.0, -50.0, 10.0}, 0.2f);
	ft::InstanceData data{};
	uint32_t  id;	(void) id;

    // Z
    data.model = glm::mat4(1.0f);
	data.model = glm::scale(data.model, {0.01f, 1.0f, 0.01f});
	data.color = {0.f, 0.0f, 0.9f};
	data.normalMatrix = glm::mat4(1.0f);
	id = _ftScene->addObjectToTheScene("models/axis.obj", data);

    // Y
    data.model = glm::rotate(glm::mat4(1), glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    data.model = glm::scale(data.model, {0.01f, 1.0f, 0.01f});
    data.color = {0.0f, 0.9f, 0.0f};
	data.normalMatrix = glm::mat4(1.0f);
	id = _ftScene->addObjectCopyToTheScene(id, data);

    // X
	data.model = glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    data.model = glm::scale(data.model, {0.01f, 1.0f, 0.01f});
	data.color = {0.9f, 0.0f, 0.0f};
	data.normalMatrix = glm::mat4(1.0f);
	_ftScene->addObjectCopyToTheScene(id, data);

	// plane
	data.model = glm::mat4(1.0f);
	data.color = {0.95f, .95f, .95f};
	data.normalMatrix = glm::mat4(1.0f);
	data.model = glm::scale(data.model, {100, 100, 100});
	_ftScene->addObjectToTheScene("models/plane.mtl.obj", data);



	data.model = glm::mat4(1.0f);
	data.color = {0.9f, 0.9f, 0.9f};
	data.normalMatrix = glm::mat4(1.0f);
	data.model = glm::rotate(data.model, glm::radians(90.0f), {1,0,0});
	id = _ftScene->addObjectToTheScene("models/viking_room.obj", data);

    auto m = _ftMaterialPool->createMaterial("textures/viking_room.png", _ftRenderer->getSampler(), _ftTexturedRdrSys->getDescriptorPool(), _ftTexturedRdrSys->getDescriptorSetLayout());
    _ftScene->addMaterialToObject(id, m);

    data.model = glm::mat4(1.0f);
	data.color = {0.9f, 0.9f, 0.9f};
	data.normalMatrix = glm::mat4(1.0f);
    data.model = glm::translate(data.model, {0, 0, 3});
    data.model = glm::rotate(data.model, glm::radians(180.0f), {1,0,0});
    data.model = glm::scale(data.model, {0.005,0.005,0.005});
	id = _ftScene->addObjectToTheScene("models/car.obj", data);

    m = _ftMaterialPool->createMaterial("textures/car.png", _ftRenderer->getSampler(), _ftTexturedRdrSys->getDescriptorPool(), _ftTexturedRdrSys->getDescriptorSetLayout());
    _ftScene->addMaterialToObject(id, m);


//    data.model = glm::mat4(1.0f);
//	data.color = {0.9f, 0.9f, 0.9f};
//	data.normalMatrix = glm::mat4(1.0f);
//	data.model = glm::scale(data.model, {2, 2, 2});
//	data.model = glm::rotate(data.model, glm::radians(180.0f), {1,0,0});
//	id = _ftScene->addObjectToTheScene("models/big_terrain.obj", data);
//
//    auto m = _ftMaterialPool->createMaterial("textures/terrain.png", _ftRenderer->getSampler());
//    _ftScene->addMaterialToObject(id, m);


//	data.model = glm::mat4(1.0f);
//	data.color = {0.9f, 0.9f, 0.9f};
//	data.normalMatrix = glm::mat4(1.0f);
////	data.model = glm::scale(data.model, {0.1, 0.1, 0.1});
//	data.model = glm::rotate(data.model, glm::radians(180.0f), {1,0,0});
//	id = _ftScene->addObjectToTheScene("models/mountain.obj", data);
//
//    m = _ftMaterialPool->createMaterial("textures/mountain.jpg", _ftRenderer->getSampler(), _ftTexturedRdrSys->getDescriptorPool(), _ftTexturedRdrSys->getDescriptorSetLayout());
//    _ftScene->addMaterialToObject(id, m);



//	data.model = glm::rotate(glm::mat4(1.0f), glm::radians(0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
//	data.model = glm::translate(data.model, glm::vec3{1.0f, -2.0f, 0.0f});
//	data.color = {0.5f, 0.95f, 0.2f};
//	id = _ftScene->addObjectCopyToTheScene(id, data);

//
//	data.model = glm::rotate(glm::mat4(1.0f), glm::radians(0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
//	data.model = glm::translate(data.model, glm::vec3{0.0f, 3.0f, 0.0f});
//	data.color = {0.7f, 0.2f, 0.4f};
//	id = _ftScene->addObjectCopyToTheScene(id, data);
//
//	id = _ftScene->addObjectToTheScene("models/smooth_vase.obj", {
//			glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f)),
//			glm::mat4(1.0f),
//			{0.95f, 0.2f, 0.0f}
//	});

//	data.model = glm::mat4(1.0f);
//	data.color = {0.95f, 0.2f, 0.0f};
//	data.normalMatrix = glm::mat4(1.0f);
//	id = _ftScene->addObjectToTheScene("models/sphere.mtl.obj", data);
//
//	data.model = glm::rotate(glm::mat4(1.0f), glm::radians(0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
//	data.model = glm::translate(data.model, glm::vec3{1.0f, -2.0f, 0.0f});
//	data.color = {0.5f, 0.95f, 0.2f};
//	id = _ftScene->addObjectCopyToTheScene(id, data);

}

void ft::Application::printFPS() {
	static auto oldTime = std::chrono::high_resolution_clock::now();
	static int fps;

	fps++;
	if (std::chrono::duration_cast<std::chrono::seconds>(std::chrono::high_resolution_clock::now() - oldTime) >= std::chrono::seconds{ 1 }) {
		oldTime = std::chrono::high_resolution_clock::now();
		std::cout << "FPS: " << fps <<  std::endl;
		fps = 0;
	}
}

// draw a frame
void ft::Application::drawFrame() {
	CommandBuffer::pointer commandBuffer;
	uint32_t index;
	std::tie(index, commandBuffer) = _ftRenderer->beginFrame();
	if (!commandBuffer) return;

	// begin the render pass
	_ftRenderer->beginRenderPass(commandBuffer, index);

	// recording the command buffer
	// for each pipeline
    vkCmdBindPipeline(commandBuffer->getVKCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, _ftSimpleRdrSys->getGraphicsPipeline()->getVKPipeline());
    vkCmdBindDescriptorSets(commandBuffer->getVKCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS,
                            _ftSimpleRdrSys->getGraphicsPipeline()->getVKPipelineLayout(), 0, 1,
                            &(_ftSimpleRdrSys->getDescriptorSets()[_currentFrame]->getVKDescriptorSet()),
                            0, nullptr);

    _ftScene->drawSimpleObjs(commandBuffer, _ftSimpleRdrSys->getGraphicsPipeline(), _currentFrame);


//	// bind the graphics pipeline
//	vkCmdBindPipeline(commandBuffer->getVKCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, _ftTexturedRdrSys->getGraphicsPipeline()->getVKPipeline());
//	// bind the descriptor sets
//	vkCmdBindDescriptorSets(commandBuffer->getVKCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS,
//							_ftTexturedRdrSys->getGraphicsPipeline()->getVKPipelineLayout(), 0, 1,
//							&(_ftTexturedRdrSys->getDescriptorSets()[_currentFrame]->getVKDescriptorSet()),
//							0, nullptr);

    _ftScene->drawTexturedObjs(commandBuffer, _ftTexturedRdrSys->getGraphicsPipeline(), _ftTexturedRdrSys, _currentFrame);

	// gui
	_ftGui->render(commandBuffer);

	// end the render pass
	_ftRenderer->endRenderPass(commandBuffer, index);
	// end the frame
	_ftRenderer->endFrame(commandBuffer, index);
}

void ft::Application::updateScene(int key) {
	if (key == _ftWindow->KEY(KeyboardKeys::KEY_UP)) {
		_ftScene->getCamera()->vRotate(-10.0f);
	} else if (key == _ftWindow->KEY(KeyboardKeys::KEY_DOWN)) {
		_ftScene->getCamera()->vRotate(10.0f);
	} else if (key == _ftWindow->KEY(KeyboardKeys::KEY_RIGHT)) {
		_ftScene->getCamera()->hRotate(10.0f);
	} else if (key == _ftWindow->KEY(KeyboardKeys::KEY_LEFT)) {
		_ftScene->getCamera()->hRotate(-10.0f);
	} else if (key == _ftWindow->KEY(KeyboardKeys::KEY_R)) {
		_ftScene->getCamera()->hardSet({5,-1,0}, {1,-1,0}, {0,1,0});
	} else if (key == _ftWindow->KEY(KeyboardKeys::KEY_W)) {
		_ftScene->getCamera()->forward(0.5f);
	} else if (key == _ftWindow->KEY(KeyboardKeys::KEY_Q)){
		_ftScene->getCamera()->translateUp(-0.5f);
	} else if (key == _ftWindow->KEY(KeyboardKeys::KEY_E)) {
		_ftScene->getCamera()->translateUp(0.5f);
	} else if (key == _ftWindow->KEY(KeyboardKeys::KEY_A)) {
		_ftScene->getCamera()->translateSide(-0.5f);
	} else if (key == _ftWindow->KEY(KeyboardKeys::KEY_S)) {
		_ftScene->getCamera()->forward(-0.5f);
	} else if (key == _ftWindow->KEY(KeyboardKeys::KEY_D)) {
		_ftScene->getCamera()->translateSide(0.5f);
	} else if (key == _ftWindow->KEY(KeyboardKeys::KEY_KP_7)) {
		_ftScene->getCamera()->rotateWorldX(10.0f);
	} else if (key == _ftWindow->KEY(KeyboardKeys::KEY_KP_9)) {
		_ftScene->getCamera()->rotateWorldX(-10.0f);
	} else if (key == _ftWindow->KEY(KeyboardKeys::KEY_KP_4)) {
		_ftScene->getCamera()->rotateWorldY(10.0f);
	} else if (key == _ftWindow->KEY(KeyboardKeys::KEY_KP_6)) {
		_ftScene->getCamera()->rotateWorldY(-10.0f);
	} else if (key == _ftWindow->KEY(KeyboardKeys::KEY_KP_1)) {
		_ftScene->getCamera()->rotateWorldZ(10.0f);
	} else if (key == _ftWindow->KEY(KeyboardKeys::KEY_KP_3)) {
		_ftScene->getCamera()->rotateWorldZ(-10.0f);
	} else if (key == _ftWindow->KEY(KeyboardKeys::KEY_Z)) {

	}
	_ftScene->updateCameraUBO();
}

