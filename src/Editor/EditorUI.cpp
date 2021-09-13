#include "Editor/EditorUI.hpp"

#include "imgui.h"
#include "ImGuizmo.h"

#include "Core/Core.hpp"

#include "Editor/EditorCore.hpp"
#include "Editor/FilePickerDialog.hpp"

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

EditorUI::EditorUI(Allocator* allocator) :
	allocator(allocator),
	renderDevice(nullptr),
	renderBackend(nullptr),
	platformBackend(nullptr)
{
	core = allocator->MakeNew<EditorCore>(allocator);
}

EditorUI::~EditorUI()
{
	allocator->MakeDelete(core);
}

void EditorUI::Initialize(Debug* debug, RenderDevice* renderDevice,
	Window* window, const ResourceManagers& resourceManagers)
{
	KOKKO_PROFILE_FUNCTION();

	this->renderDevice = renderDevice;

	renderBackend = allocator->MakeNew<ImGuiRenderBackend>();
	platformBackend = allocator->MakeNew<ImGuiPlatformBackend>();

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

	ImGui::StyleColorsDark();

	renderBackend->Initialize();

	InputManager* inputManager = window->GetInputManager();
	GLFWwindow* glfwWindow = window->GetGlfwWindow();

	platformBackend->Initialize(glfwWindow, inputManager->GetImGuiInputView());

	core->Initialize(debug, renderDevice, inputManager, resourceManagers);
}

void EditorUI::Deinitialize()
{
	KOKKO_PROFILE_FUNCTION();

	platformBackend->Deinitialize();
	renderBackend->Deinitialize();

	ImGui::DestroyContext();

	allocator->MakeDelete(platformBackend);
	allocator->MakeDelete(renderBackend);
}

void EditorUI::StartFrame()
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

void EditorUI::Update(EngineSettings* engineSettings, World* world, bool& shouldExitOut)
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

void EditorUI::DrawSceneView()
{
	KOKKO_PROFILE_FUNCTION();

	core->DrawSceneView();
}

void EditorUI::EndFrame()
{
	KOKKO_PROFILE_FUNCTION();

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

const Framebuffer& EditorUI::GetSceneViewFramebuffer()
{
	return core->GetSceneViewFramebuffer();
}

CameraParameters EditorUI::GetEditorCameraParameters() const
{
	return core->GetEditorCameraParameters();
}

