#include "Editor/EditorUI.hpp"

#include "imgui.h"

#include "Core/Core.hpp"

#include "Editor/EditorViews.hpp"
#include "Editor/FilePickerDialog.hpp"

#include "Engine/World.hpp"

#include "Entity/EntityManager.hpp"

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
	renderBackend(nullptr),
	platformBackend(nullptr)
{
	views = allocator->MakeNew<EditorViews>();

}

EditorUI::~EditorUI()
{
	allocator->MakeDelete(views);
}

void EditorUI::Initialize(RenderDevice* renderDevice, Window* window, const ResourceManagers& resourceManagers)
{
	KOKKO_PROFILE_FUNCTION();

	this->renderDevice = renderDevice;

	renderBackend = allocator->MakeNew<ImGuiRenderBackend>();
	platformBackend = allocator->MakeNew<ImGuiPlatformBackend>();

	InputManager* inputManager = window->GetInputManager();

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

	ImGui::StyleColorsDark();

	renderBackend->Initialize();

	platformBackend->Initialize(window->GetGlfwWindow(), inputManager->GetImGuiInputView());

	views->entityView.Initialize(resourceManagers);
	views->sceneView.Initialize(renderDevice, inputManager);
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

	views->sceneView.ResizeFramebufferIfRequested();

	renderBackend->NewFrame();
	platformBackend->NewFrame();

	ImGui::NewFrame();

	ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());
}

void EditorUI::Update(World* world, bool& shouldExitOut)
{
	KOKKO_PROFILE_FUNCTION();

	views->sceneView.Update();

	Draw(world, shouldExitOut);

	static bool firstRun = true;
	if (firstRun)
	{
		firstRun = false;
		world->LoadFromFile("res/scenes/default.level", "default.level");
	}
}

void EditorUI::DrawSceneView()
{
	views->sceneView.Draw();
}

void EditorUI::EndFrame()
{
	KOKKO_PROFILE_FUNCTION();

	ImGui::Render();

	// Bind default framebuffer
	renderDevice->BindFramebuffer(RenderFramebufferTarget::Framebuffer, 0);

	RenderCommandData::ClearColorData clearColor{ 0.0f, 0.0f, 0.0f, 1.0f };
	renderDevice->ClearColor(&clearColor);

	RenderCommandData::ClearMask clearMask{ true, false, false };
	renderDevice->Clear(&clearMask);

	renderBackend->RenderDrawData(ImGui::GetDrawData());
}

const Framebuffer& EditorUI::GetSceneViewFramebuffer()
{
	return views->sceneView.GetFramebuffer();
}

CameraParameters EditorUI::GetEditorCameraParameters() const
{
	return views->sceneView.GetCameraParameters();
}

void EditorUI::Draw(World* world, bool& shouldExitOut)
{
	DrawMainMenuBar(world, shouldExitOut);

	views->entityView.Draw(world);

	ImGui::ShowDemoWindow();
}

void EditorUI::DrawMainMenuBar(World* world, bool& shouldExitOut)
{
	bool openLevel = false, saveLevel = false;

	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("New"))
			{
				world->ClearAllEntities();
			}

			if (ImGui::MenuItem("Open..."))
			{
				openLevel = true;
			}

			if (ImGui::MenuItem("Save as..."))
			{
				saveLevel = true;
			}

			ImGui::Separator();

			shouldExitOut = ImGui::MenuItem("Exit");

			ImGui::EndMenu();
		}
		ImGui::EndMainMenuBar();
	}

	if (openLevel)
		views->filePicker.StartDialogFileOpen("Open level", "Open");

	if (saveLevel)
		views->filePicker.StartDialogFileSave("Save level as", "Save");

	std::filesystem::path filePickerPathOut;
	bool filePickerClosed = views->filePicker.Update(filePickerPathOut);

	if (filePickerClosed && filePickerPathOut.empty() == false)
	{
		std::string pathStr = filePickerPathOut.u8string();
		std::string filenameStr = filePickerPathOut.filename().u8string();

		FilePickerDialog::DialogType type = views->filePicker.GetLastDialogType();
		if (type == FilePickerDialog::DialogType::FileOpen)
		{
			world->ClearAllEntities();
			world->LoadFromFile(pathStr.c_str(), filenameStr.c_str());
		}
		else if (type == FilePickerDialog::DialogType::FileSave)
		{
			world->WriteToFile(pathStr.c_str(), filenameStr.c_str());
		}
	}
}
