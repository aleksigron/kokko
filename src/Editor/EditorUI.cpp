#include "Editor/EditorUI.hpp"

#include "imgui.h"

#include "Core/Core.hpp"

#include "Editor/EditorViews.hpp"
#include "Editor/FilePickerDialog.hpp"

#include "Engine/Engine.hpp"

#include "Graphics/World.hpp"

#include "Math/Rectangle.hpp"

#include "Memory/Allocator.hpp"

#include "System/ImGuiRenderBackend.hpp"
#include "System/ImGuiPlatformBackend.hpp"
#include "System/InputManager.hpp"
#include "System/Window.hpp"

EditorUI::EditorUI(Allocator* allocator) :
	allocator(allocator),
	renderBackend(nullptr),
	platformBackend(nullptr),
	world(nullptr)
{
	views = allocator->MakeNew<EditorViews>();
}

EditorUI::~EditorUI()
{
	allocator->MakeDelete(views);
}

void EditorUI::Initialize(Engine* engine)
{
	KOKKO_PROFILE_FUNCTION();

	renderBackend = allocator->MakeNew<ImGuiRenderBackend>();
	platformBackend = allocator->MakeNew<ImGuiPlatformBackend>();

	this->world = engine->GetWorld();

	Window* window = engine->GetMainWindow();
	InputManager* inputManager = window->GetInputManager();

	editorCamera.SetInputManager(inputManager);

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;

	ImGui::StyleColorsDark();

	renderBackend->Initialize();

	platformBackend->Initialize(window->GetGlfwWindow(), inputManager->GetImGuiInputView());

	views->entityView.Initialize(engine);
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

void EditorUI::Update()
{
	KOKKO_PROFILE_FUNCTION();

	editorCamera.Update();

	DrawMainMenuBar();

	views->entityView.Draw();

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

void EditorUI::DrawMainMenuBar()
{
	bool openLevel = false, saveLevel = false;

	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("Open level"))
			{
				openLevel = true;
			}

			if (ImGui::MenuItem("Save level"))
			{
				saveLevel = true;
			}
			ImGui::EndMenu();
		}
		ImGui::EndMainMenuBar();
	}

	if (openLevel)
		views->filePicker.StartDialogFileOpen("Open level", "Open");

	if (saveLevel)
		views->filePicker.StartDialogFileSave("Save level as", "Save");

	String filePickerPathOut(allocator);
	bool filePickerClosed = views->filePicker.Update(filePickerPathOut);

	if (filePickerClosed && filePickerPathOut.GetLength() > 0)
	{
		FilePickerDialog::DialogType type = views->filePicker.GetLastDialogType();
		if (type == FilePickerDialog::DialogType::FileOpen)
		{
			// TODO: Clear world before loading new
			world->LoadFromFile(filePickerPathOut.GetCStr());
		}
		else if (type == FilePickerDialog::DialogType::FileSave)
		{
			world->WriteToFile(filePickerPathOut.GetCStr());
		}
	}
}
