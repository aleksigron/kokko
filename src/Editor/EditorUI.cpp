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

	InputManager* inputManager = window->GetInputManager();

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

	ImGui::StyleColorsDark();

	editorWindows[EditorWindow_Entities] = EditorWindowInfo{ "Entities", true, false };
	editorWindows[EditorWindow_Properties] = EditorWindowInfo{ "Properties", true, false };
	editorWindows[EditorWindow_Scene] = EditorWindowInfo{ "Scene", true, false };
	editorWindows[EditorWindow_Debug] = EditorWindowInfo{ "Debug", true, false };

	renderBackend->Initialize();

	platformBackend->Initialize(window->GetGlfwWindow(), inputManager->GetImGuiInputView());

	core->entityView.Initialize(resourceManagers);
	core->sceneView.Initialize(renderDevice, inputManager);
	core->debugView.Initialize(debug);
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

	core->sceneView.ResizeFramebufferIfRequested();

	renderBackend->NewFrame();
	platformBackend->NewFrame();

	ImGui::NewFrame();
	ImGuizmo::BeginFrame();

	ImGuizmo::SetOrthographic(false);

	ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());
}

void EditorUI::Update(World* world, bool& shouldExitOut)
{
	KOKKO_PROFILE_FUNCTION();

	core->sceneView.Update();

	DrawMainMenuBar(world, shouldExitOut);

	core->entityListView.Draw(editorWindows[EditorWindow_Entities], core->selectionContext, world);
	core->entityView.Draw(editorWindows[EditorWindow_Properties], core->selectionContext, world);
	core->debugView.Draw(editorWindows[EditorWindow_Debug]);

	ImGui::ShowDemoWindow();

	static bool firstRun = true;
	if (firstRun)
	{
		firstRun = false;
		world->LoadFromFile("res/scenes/default.level", "default.level");
	}
}

void EditorUI::DrawSceneView(World* world)
{
	KOKKO_PROFILE_FUNCTION();

	core->sceneView.Draw(editorWindows[EditorWindow_Scene], world, core->selectionContext);
}

void EditorUI::EndFrame()
{
	KOKKO_PROFILE_FUNCTION();

	for (size_t i = 0; i < EditorWindow_COUNT; ++i)
		editorWindows[i].requestFocus = false;

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
	return core->sceneView.GetFramebuffer();
}

CameraParameters EditorUI::GetEditorCameraParameters() const
{
	return core->sceneView.GetCameraParameters();
}

void EditorUI::DrawMainMenuBar(World* world, bool& shouldExitOut)
{
	KOKKO_PROFILE_FUNCTION();

	bool openLevel = false, saveLevel = false;

	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("New"))
				world->ClearAllEntities();

			if (ImGui::MenuItem("Open..."))
				openLevel = true;

			if (ImGui::MenuItem("Save as..."))
				saveLevel = true;

			ImGui::Separator();

			shouldExitOut = ImGui::MenuItem("Exit");

			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Edit"))
		{
			if (ImGui::MenuItem("Copy"))
			{
				core->CopyEntity(world);
			}

			if (ImGui::MenuItem("Paste"))
			{
				core->PasteEntity(world);
			}

			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("View"))
		{
			for (size_t i = 0; i < EditorWindow_COUNT; ++i)
			{
				EditorWindowInfo& windowInfo = editorWindows[i];

				if (ImGui::MenuItem(windowInfo.title, nullptr))
				{
					windowInfo.isOpen = true;
					windowInfo.requestFocus = true;
				}
			}

			ImGui::EndMenu();
		}

		ImGui::EndMainMenuBar();
	}

	if (openLevel)
		core->filePicker.StartDialogFileOpen("Open level", "Open");

	if (saveLevel)
		core->filePicker.StartDialogFileSave("Save level as", "Save");

	std::filesystem::path filePickerPathOut;
	bool filePickerClosed = core->filePicker.Update(filePickerPathOut);

	if (filePickerClosed && filePickerPathOut.empty() == false)
	{
		std::string pathStr = filePickerPathOut.u8string();
		std::string filenameStr = filePickerPathOut.filename().u8string();

		FilePickerDialog::DialogType type = core->filePicker.GetLastDialogType();
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
