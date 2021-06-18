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

#include "Rendering/CameraSystem.hpp"
#include "Rendering/LightManager.hpp"
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

	Vec3f position(-3.0f, 2.0f, 6.0f);
	Vec3f target(0.0f, 1.0f, 0.0f);
	editorCamera.LookAt(position, target);
}

EditorUI::~EditorUI()
{
	allocator->MakeDelete(views);
}

void EditorUI::Initialize(Window* window, const ResourceManagers& resourceManagers)
{
	KOKKO_PROFILE_FUNCTION();

	renderBackend = allocator->MakeNew<ImGuiRenderBackend>();
	platformBackend = allocator->MakeNew<ImGuiPlatformBackend>();

	InputManager* inputManager = window->GetInputManager();

	editorCamera.SetInputManager(inputManager);

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;

	ImGui::StyleColorsDark();

	renderBackend->Initialize();

	platformBackend->Initialize(window->GetGlfwWindow(), inputManager->GetImGuiInputView());

	views->entityView.Initialize(resourceManagers);
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

	renderBackend->NewFrame();
	platformBackend->NewFrame();

	ImGui::NewFrame();
}

void EditorUI::Update(World* world, bool& shouldExitOut)
{
	KOKKO_PROFILE_FUNCTION();

	editorCamera.Update();

	DrawMainMenuBar(world, shouldExitOut);

	views->entityView.Draw(world);

	ImGui::ShowDemoWindow();
}

void EditorUI::EndFrame()
{
	KOKKO_PROFILE_FUNCTION();

	ImGui::Render();
	renderBackend->RenderDrawData(ImGui::GetDrawData());
}

ViewRectangle EditorUI::GetWorldViewport()
{
	ImGuiViewport* viewport = ImGui::GetMainViewport();

	Vec2i pos = Vec2f(viewport->WorkPos.x, viewport->WorkPos.y).As<int>();
	Vec2i size = Vec2f(viewport->WorkSize.x, viewport->WorkSize.y).As<int>();
	Vec2i fullSize = Vec2f(viewport->Size.x, viewport->Size.y).As<int>();

	ViewRectangle rect;
	rect.position = fullSize - size - pos;
	rect.size = size;

	return rect;
}

Mat4x4fBijection EditorUI::GetEditorCameraTransform() const
{
	return editorCamera.GetCameraTransform();
}

ProjectionParameters EditorUI::GetEditorCameraProjection() const
{
	return editorCamera.GetProjectionParameters();
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
