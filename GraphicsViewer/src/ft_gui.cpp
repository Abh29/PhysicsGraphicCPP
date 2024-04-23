#include "../includes/ft_gui.h"
#include "../imgui/imgui_internal.h"

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

// Helper to wire demo markers located in code to an interactive browser
typedef void (*ImGuiDemoMarkerCallback)(const char* file, int line, const char* section, void* user_data);
extern ImGuiDemoMarkerCallback      GImGuiDemoMarkerCallback;
extern void*                        GImGuiDemoMarkerCallbackUserData;
#define IMGUI_DEMO_MARKER(section)  do { if (GImGuiDemoMarkerCallback != NULL) GImGuiDemoMarkerCallback(__FILE__, __LINE__, section, GImGuiDemoMarkerCallbackUserData); } while (0)


// Helper to display a little (?) mark which shows a tooltip when hovered.
// In your own code you may want to display an actual icon if you are using a merged icon fonts (see docs/FONTS.md)
[[maybe_unused]]static void HelpMarker(const char* desc)
{
    ImGui::TextDisabled("(?)");
    if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayShort) && ImGui::BeginTooltip())
    {
        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
        ImGui::TextUnformatted(desc);
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
}

void ft::Gui::showExampleMenuFile()
{
    IMGUI_DEMO_MARKER("Examples/Menu");
    if (ImGui::MenuItem("New")) {}
    if (ImGui::MenuItem("Open", "Ctrl+O")) {}
    if (ImGui::MenuItem("Save", "Ctrl+S")) {}
    if (ImGui::MenuItem("Save As..")) {}
    ImGui::Separator();
    if (ImGui::MenuItem("New Model")) {}
    if (ImGui::MenuItem("New Texture")) {}

    ImGui::Separator();
    IMGUI_DEMO_MARKER("Examples/Menu/Options");
    if (ImGui::BeginMenu("Options"))
    {
        static bool enabled = true;
        ImGui::MenuItem("Enabled", "", &enabled);
        ImGui::BeginChild("child", ImVec2(0, 60), true);
        for (int i = 0; i < 10; i++)
            ImGui::Text("Scrolling Text %d", i);
        ImGui::EndChild();
        static float f = 0.5f;
        static int n = 0;
        ImGui::SliderFloat("Value", &f, 0.0f, 1.0f);
        ImGui::InputFloat("Input", &f, 0.1f);
        ImGui::Combo("Combo", &n, "Yes\0No\0Maybe\0\0");
        ImGui::EndMenu();
    }

    IMGUI_DEMO_MARKER("Examples/Menu/Colors");
    if (ImGui::BeginMenu("Colors"))
    {
        float sz = ImGui::GetTextLineHeight();
        for (int i = 0; i < ImGuiCol_COUNT; i++)
        {
            const char* name = ImGui::GetStyleColorName((ImGuiCol)i);
            ImVec2 p = ImGui::GetCursorScreenPos();
            ImGui::GetWindowDrawList()->AddRectFilled(p, ImVec2(p.x + sz, p.y + sz), ImGui::GetColorU32((ImGuiCol)i));
            ImGui::Dummy(ImVec2(sz, sz));
            ImGui::SameLine();
            ImGui::MenuItem(name);
        }
        ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("Disabled", false)) // Disabled
    {
        IM_ASSERT(0);
    }
    ImGui::Separator();
    if (ImGui::MenuItem("Quit", "Alt+F4")) {
        _ftWindow->close();
    }
}


void ft::Gui::newFrame() {
	ImGui_ImplVulkan_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
}

void ft::Gui::showDemo() {
	ImGui::ShowDemoWindow();
}

void ft::Gui::showGUI(uint32_t flags, bool *p_open) {
    (void)flags; (void)p_open;
    showTitleBar();
    showMainMenue();
    if (show_app_metrics)
        showMetrics(&show_app_metrics);
}

void ft::Gui::render(ft::CommandBuffer::pointer commandBuffer) {
	ImGui::Render();
	ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer->getVKCommandBuffer());
}

void ft::Gui::showMetrics(bool *p_open) {
    auto context = ImGui::GetCurrentContext();

    ImGuiIO& io = context->IO;
    ImGuiMetricsConfig* cfg = &context->DebugMetricsConfig; (void)cfg;

    ImGuiWindowFlags window_flags = 0;
    window_flags |= ImGuiWindowFlags_NoScrollbar;
//    window_flags |= ImGuiWindowFlags_NoMove;
    window_flags |= ImGuiWindowFlags_NoResize;
    window_flags |= ImGuiWindowFlags_NoCollapse;
    window_flags |= ImGuiWindowFlags_NoNav;


    const ImGuiViewport* main_viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(ImVec2(0, main_viewport->Size.y - 65), ImGuiCond_Always);


    // Main body of the Demo window starts here.
    if (!ImGui::Begin("Metrics", p_open, window_flags))
    {
        ImGui::End();
        return;
    }

    // Basic info
    ImGui::Text("Dear ImGui %s", ImGui::GetVersion());
    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);

    ImGui::End();
}

void ft::Gui::showTitleBar() {
    IM_ASSERT(ImGui::GetCurrentContext() != NULL && "Missing dear imgui context. Refer to examples app!");

    static bool no_titlebar = true;
    static bool no_scrollbar = false;
    static bool no_menu = false;
    static bool no_move = false;
    static bool no_resize = true;
    static bool no_collapse = false;
    static bool no_nav = false;
    static bool no_background = false;
    static bool no_bring_to_front = false;
    static bool unsaved_document = false;

    ImGuiWindowFlags window_flags = 0;
    if (no_titlebar)        window_flags |= ImGuiWindowFlags_NoTitleBar;
    if (no_scrollbar)       window_flags |= ImGuiWindowFlags_NoScrollbar;
    if (!no_menu)           window_flags |= ImGuiWindowFlags_MenuBar;
    if (no_move)            window_flags |= ImGuiWindowFlags_NoMove;
    if (no_resize)          window_flags |= ImGuiWindowFlags_NoResize;
    if (no_collapse)        window_flags |= ImGuiWindowFlags_NoCollapse;
    if (no_nav)             window_flags |= ImGuiWindowFlags_NoNav;
    if (no_background)      window_flags |= ImGuiWindowFlags_NoBackground;
    if (no_bring_to_front)  window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus;
    if (unsaved_document)   window_flags |= ImGuiWindowFlags_UnsavedDocument;

    // We specify a default position/size in case there's no data in the .ini file.
    // We only do it to make the demo applications a little more welcoming, but typically this isn't required.
    const ImGuiViewport* main_viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(main_viewport->Size.x, 30), ImGuiCond_Always);

    // Main body of the Demo window starts here.
    if (!ImGui::Begin("FTGraphics", nullptr, window_flags))
    {
        // Early out if the window is collapsed, as an optimization.
        ImGui::End();
        return;
    }

    // Most "big" widgets share a common width settings by default. See 'Demo->Layout->Widgets Width' for details.
    // e.g. Use 2/3 of the space for widgets and 1/3 for labels (right align)
    //ImGui::PushItemWidth(-ImGui::GetWindowWidth() * 0.35f);
    // e.g. Leave a fixed amount of width for labels (by passing a negative value), the rest goes to widgets.
    ImGui::PushItemWidth(ImGui::GetFontSize() * -12);

    // Menu Bar
    if (ImGui::BeginMenuBar())
    {
        if (ImGui::BeginMenu("Menu"))
        {
            IMGUI_DEMO_MARKER("Menu/File");
            showExampleMenuFile();
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Examples"))
        {
            IMGUI_DEMO_MARKER("Menu/Examples");
//            ImGui::MenuItem("Main menu bar", NULL, &show_app_main_menu_bar);
//            ImGui::MenuItem("Console", NULL, &show_app_console);
//            ImGui::MenuItem("Log", NULL, &show_app_log);
//            ImGui::MenuItem("Simple layout", NULL, &show_app_layout);
//            ImGui::MenuItem("Property editor", NULL, &show_app_property_editor);
//            ImGui::MenuItem("Long text display", NULL, &show_app_long_text);
//            ImGui::MenuItem("Auto-resizing window", NULL, &show_app_auto_resize);
//            ImGui::MenuItem("Constrained-resizing window", NULL, &show_app_constrained_resize);
//            ImGui::MenuItem("Simple overlay", NULL, &show_app_simple_overlay);
//            ImGui::MenuItem("Fullscreen window", NULL, &show_app_fullscreen);
//            ImGui::MenuItem("Manipulating window titles", NULL, &show_app_window_titles);
//            ImGui::MenuItem("Custom rendering", NULL, &show_app_custom_rendering);
//            ImGui::MenuItem("Documents", NULL, &show_app_documents);
            ImGui::EndMenu();
        }
        //if (ImGui::MenuItem("MenuItem")) {} // You can also use MenuItem() inside a menu bar!
        if (ImGui::BeginMenu("Tools"))
        {
            IMGUI_DEMO_MARKER("Menu/Tools");
            ImGui::MenuItem("Metrics", NULL, &show_app_metrics, true);
//            ImGui::MenuItem("Debug Log", NULL, &show_app_debug_log, has_debug_tools);
//            ImGui::MenuItem("Stack Tool", NULL, &show_app_stack_tool, has_debug_tools);
//            ImGui::MenuItem("Style Editor", NULL, &show_app_style_editor);
            ImGui::MenuItem("About", NULL, &show_app_about);
            ImGui::EndMenu();
        }
        ImGui::EndMenuBar();
    }

    // End of ShowDemoWindow()
    ImGui::PopItemWidth();
    ImGui::End();
}

void ft::Gui::showMainMenue() {
    IM_ASSERT(ImGui::GetCurrentContext() != NULL && "Missing dear imgui context. Refer to examples app!");

    ImGuiWindowFlags window_flags = 0;
    window_flags |= ImGuiWindowFlags_NoTitleBar;
    window_flags |= ImGuiWindowFlags_NoScrollbar;
    window_flags |= ImGuiWindowFlags_NoMove;
    window_flags |= ImGuiWindowFlags_NoResize;
    window_flags |= ImGuiWindowFlags_NoCollapse;
    window_flags |= ImGuiWindowFlags_NoNav;


    // We specify a default position/size in case there's no data in the .ini file.
    // We only do it to make the demo applications a little more welcoming, but typically this isn't required.
    const ImGuiViewport* main_viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(ImVec2(main_viewport->Size.x - 200, 31), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(200, main_viewport->Size.y - 30), ImGuiCond_Always);

    // Main body of the Demo window starts here.
    if (!ImGui::Begin("main_menue", nullptr, window_flags))
    {
        // Early out if the window is collapsed, as an optimization.
        ImGui::End();
        return;
    }

    // Most "big" widgets share a common width settings by default. See 'Demo->Layout->Widgets Width' for details.
    // e.g. Use 2/3 of the space for widgets and 1/3 for labels (right align)
    //ImGui::PushItemWidth(-ImGui::GetWindowWidth() * 0.35f);
    // e.g. Leave a fixed amount of width for labels (by passing a negative value), the rest goes to widgets.
    ImGui::PushItemWidth(ImGui::GetFontSize() * -12);



    // End of ShowDemoWindow()
    ImGui::PopItemWidth();
    ImGui::End();
}

