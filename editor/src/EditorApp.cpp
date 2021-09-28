#include "EditorApp.hpp"

#include <cmath>

#include "imgui.h"
#include "ImGuizmo.h"

#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

#include "EditorCore.hpp"
#include "EditorUserSettings.hpp"
#include "FilePickerDialog.hpp"

#include "Core/Core.hpp"

#include "Engine/Engine.hpp"
#include "Engine/EntityManager.hpp"
#include "Engine/World.hpp"

#include "Graphics/Scene.hpp"

#include "Math/Math.hpp"
#include "Math/Rectangle.hpp"

#include "Memory/Allocator.hpp"

#include "Rendering/CameraParameters.hpp"
#include "Rendering/CameraSystem.hpp"
#include "Rendering/LightManager.hpp"
#include "Rendering/RenderDevice.hpp"
#include "Rendering/Renderer.hpp"

#include "Resources/ResourceManagers.hpp"

#include "System/InputManager.hpp"
#include "System/Window.hpp"

EditorApp::EditorApp(Allocator* allocator) :
	allocator(allocator),
	renderDevice(nullptr),
	core(nullptr),
	project(allocator)
{
	core = allocator->MakeNew<EditorCore>(allocator);
}

EditorApp::~EditorApp()
{
	allocator->MakeDelete(core);
}

void EditorApp::Initialize(Engine* engine)
{
	KOKKO_PROFILE_FUNCTION();

	this->renderDevice = engine->GetRenderDevice();

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

	float scale = engine->GetMainWindow()->GetScreenCoordinateScale();

	// TODO: Scale spacing values
	// TODO: Update font size and spacing if window is moved to another screen

	{
		KOKKO_PROFILE_SCOPE("ImGui AddFontFromFileTTF()");

		float fontSize = std::floor(15.0f * scale);
		io.Fonts->AddFontFromFileTTF("res/fonts/roboto/Roboto-Regular.ttf", fontSize);
	}

	ImGui::StyleColorsDark();

	// Update style colors to linear space
	ImVec4* imguiColors = ImGui::GetStyle().Colors;
	for (size_t i = 0; i < ImGuiCol_COUNT; ++i)
	{
		ImVec4 color = imguiColors[i];
		color.x = Math::SrgbFloatToLinear(color.x);
		color.y = Math::SrgbFloatToLinear(color.y);
		color.z = Math::SrgbFloatToLinear(color.z);
		// Leave alpha as is

		imguiColors[i] = color;
	}

	GLFWwindow* glfwWindow = engine->GetMainWindow()->GetGlfwWindow();

	{
		KOKKO_PROFILE_SCOPE("ImGui_ImplGlfw_InitForOpenGL()");
		ImGui_ImplGlfw_InitForOpenGL(glfwWindow, true);
	}

	{
		KOKKO_PROFILE_SCOPE("ImGui_ImplOpenGL3_Init()");
		ImGui_ImplOpenGL3_Init();
	}

	EditorUserSettings userSettings;
	if (userSettings.DeserializeFromFile("editor_user_settings.yml"))
	{
		if (userSettings.lastOpenedProject.empty())
		{
			KK_LOG_INFO("Last opened project is empty, should open project dialog.");
		}
		else if (project.DeserializeFromFile(userSettings.lastOpenedProject))
		{
			std::string path = project.GetRootPath();
			const String& name = project.GetName();

			KK_LOG_INFO("Opened editor project from {}, named {}", path.c_str(), name.GetCStr());
		}
		else
		{
			std::string pathStr = userSettings.lastOpenedProject.u8string();
			KK_LOG_ERROR("Failed to open project.yml from path {}", pathStr.c_str());
		}
	}
	else
	{
		KK_LOG_INFO("Failed to open editor_user_settings.yml, should open project dialog.");
	}

	core->Initialize(engine);
}

void EditorApp::Deinitialize()
{
	KOKKO_PROFILE_FUNCTION();

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();

	ImGui::DestroyContext();
}

void EditorApp::StartFrame()
{
	KOKKO_PROFILE_FUNCTION();

	core->ResizeSceneViewFramebufferIfRequested();

	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();

	ImGui::NewFrame();
	ImGuizmo::BeginFrame();

	ImGuizmo::SetOrthographic(false);

	ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());
}

void EditorApp::Update(EngineSettings* engineSettings, World* world, bool& shouldExitOut)
{
	KOKKO_PROFILE_FUNCTION();

	core->SetWorld(world);
	core->Update(engineSettings);

	//ImGui::ShowDemoWindow();

	static bool firstRun = true;
	if (firstRun)
	{
		firstRun = false;
		world->LoadFromFile("res/scenes/default.level", "default.level");
	}

	if (core->IsExitRequested())
		shouldExitOut = true;
}

void EditorApp::EndFrame()
{
	KOKKO_PROFILE_FUNCTION();

	core->DrawSceneView();

	core->EndFrame();

	{
		KOKKO_PROFILE_SCOPE("ImGui::Render()");
		ImGui::Render();
	}

	{
		KOKKO_PROFILE_SCOPE("Clear default framebuffer");

		renderDevice->BindFramebuffer(RenderFramebufferTarget::Framebuffer, 0);

		RenderCommandData::ClearColorData clearColor{ 0.0f, 0.0f, 0.0f, 1.0f };
		renderDevice->ClearColor(&clearColor);

		RenderCommandData::ClearMask clearMask{ true, false, false };
		renderDevice->Clear(&clearMask);
	}

	{
		KOKKO_PROFILE_SCOPE("ImGuiRenderBackend::RenderDrawData()");
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	}
}

const Framebuffer& EditorApp::GetSceneViewFramebuffer()
{
	return core->GetSceneViewFramebuffer();
}

CameraParameters EditorApp::GetEditorCameraParameters() const
{
	return core->GetEditorCameraParameters();
}

