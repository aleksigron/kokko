#include "EditorApp.hpp"

#include "imgui.h"
#include "ImGuizmo.h"

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

#include "System/ImGuiRenderBackend.hpp"
#include "System/ImGuiPlatformBackend.hpp"
#include "System/InputManager.hpp"
#include "System/Window.hpp"

EditorApp::EditorApp(Allocator* allocator) :
	allocator(allocator),
	renderDevice(nullptr),
	renderBackend(nullptr),
	platformBackend(nullptr)
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

	renderBackend = allocator->MakeNew<ImGuiRenderBackend>();
	platformBackend = allocator->MakeNew<ImGuiPlatformBackend>();

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

	ImGui::StyleColorsDark();

	renderBackend->Initialize();

	Window* window = engine->GetMainWindow();
	InputManager* inputManager = window->GetInputManager();
	GLFWwindow* glfwWindow = window->GetGlfwWindow();

	platformBackend->Initialize(glfwWindow, inputManager->GetImGuiInputView());

	core->Initialize(engine);
}

void EditorApp::Deinitialize()
{
	KOKKO_PROFILE_FUNCTION();

	platformBackend->Deinitialize();
	renderBackend->Deinitialize();

	ImGui::DestroyContext();

	allocator->MakeDelete(platformBackend);
	allocator->MakeDelete(renderBackend);
}

void EditorApp::StartFrame()
{
	KOKKO_PROFILE_FUNCTION();

	core->ResizeSceneViewFramebufferIfRequested();

	renderBackend->NewFrame();
	platformBackend->NewFrame();

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
		renderBackend->RenderDrawData(ImGui::GetDrawData());
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

