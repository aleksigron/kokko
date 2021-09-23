#include "EditorApp.hpp"

#include "imgui.h"
#include "ImGuizmo.h"

#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

#include "Core/Core.hpp"

#include "EditorCore.hpp"
#include "FilePickerDialog.hpp"

#include "Engine/Engine.hpp"
#include "Engine/EntityManager.hpp"
#include "Engine/World.hpp"

#include "Graphics/Scene.hpp"

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
	renderDevice(nullptr)
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

	ImGui::StyleColorsDark();

	GLFWwindow* glfwWindow = engine->GetMainWindow()->GetGlfwWindow();

	ImGui_ImplGlfw_InitForOpenGL(glfwWindow, true);
	ImGui_ImplOpenGL3_Init();

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

		// Bind default framebuffer
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

