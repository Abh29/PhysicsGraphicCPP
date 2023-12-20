#include "../include.h"

ft::Gui::Gui(Instance::pointer instance, PhysicalDevice::pointer physicalDevice,
			 Device::pointer device, Window::pointer window,
			 RenderPass::pointer renderPass, uint32_t imageCount) :
			 _ftInstance(instance), _ftPhysicalDevice(physicalDevice), _ftDevice(device),
			 _ftWindow(window), _ftRenderPass(renderPass) {

		//1: create descriptor pool for IMGUI
			VkDescriptorPoolSize pool_sizes[] =
			{
					{ VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
					{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
					{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
					{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
					{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
					{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
					{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
					{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
					{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
					{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
					{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
			};

			VkDescriptorPoolCreateInfo pool_info = {};
			pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
			pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
			pool_info.maxSets = 1000;
			pool_info.poolSizeCount = std::size(pool_sizes);
			pool_info.pPoolSizes = pool_sizes;

			if (vkCreateDescriptorPool(_ftDevice->getVKDevice(), &pool_info, nullptr, &_descriptorPool) != VK_SUCCESS) {
				throw std::runtime_error("can not create a descriptor pool for imgui!");
			}

			// 2: initialize imgui library for glfw
			ImGui::CreateContext();
			ImGui::StyleColorsDark();
			ImGui_ImplGlfw_InitForVulkan((GLFWwindow*)_ftWindow->getRawWindowPointer(), true);

			// init imgui for vulkan
			ImGui_ImplVulkan_InitInfo init_info = {};
			init_info.Instance = _ftInstance->getVKInstance();
			init_info.PhysicalDevice = _ftPhysicalDevice->getVKPhysicalDevice();
			init_info.Device = _ftDevice->getVKDevice();
			init_info.QueueFamily = _ftDevice->getQueueFamilyIndices().graphicsFamily.value();
			init_info.Queue = _ftDevice->getVKGraphicsQueue();
			init_info.DescriptorPool = _descriptorPool;
			init_info.Subpass = 0;
			init_info.MinImageCount = imageCount;
			init_info.ImageCount = imageCount;
			init_info.MSAASamples = _ftDevice->getMSAASamples();
			ImGui_ImplVulkan_Init(&init_info, _ftRenderPass->getVKRenderPass());

			// load fonts
			CommandBuffer::pointer commandBuffer = std::make_shared<CommandBuffer>(_ftDevice);
			commandBuffer->beginRecording(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
			ImGui_ImplVulkan_CreateFontsTexture(commandBuffer->getVKCommandBuffer());
			commandBuffer->end();
			commandBuffer->submit(_ftDevice->getVKGraphicsQueue());

			// clear font textures from host memory
			ImGui_ImplVulkan_DestroyFontUploadObjects();

}


ft::Gui::~Gui() {
	vkDestroyDescriptorPool(_ftDevice->getVKDevice(), _descriptorPool, nullptr);
	ImGui_ImplVulkan_Shutdown();
}


void ft::Gui::newFrame() {
	ImGui_ImplVulkan_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
}

void ft::Gui::showDemo() {
	ImGui::ShowDemoWindow();
}

void ft::Gui::render(ft::CommandBuffer::pointer commandBuffer) {
	ImGui::Render();
	ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer->getVKCommandBuffer());
}