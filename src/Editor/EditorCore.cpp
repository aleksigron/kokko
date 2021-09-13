#include "Editor/EditorCore.hpp"

#include "imgui.h"

#include "Core/Core.hpp"

#include "Engine/World.hpp"

#include "Rendering/CameraParameters.hpp"

#include "Resources/LevelSerializer.hpp"

EditorCore::EditorCore(Allocator* allocator) :
	exitRequested(false),
	world(nullptr),
	copiedEntity(allocator),
	selectionContext{}
{
	editorWindows[EditorWindow_Entities] = EditorWindowInfo{ "Entities", true, false };
	editorWindows[EditorWindow_Properties] = EditorWindowInfo{ "Properties", true, false };
	editorWindows[EditorWindow_Scene] = EditorWindowInfo{ "Scene", true, false };
	editorWindows[EditorWindow_Debug] = EditorWindowInfo{ "Debug", true, false };
}

void EditorCore::Initialize(Debug* debug, RenderDevice* renderDevice, InputManager* inputManager, const ResourceManagers& resourceManagers)
{
	entityView.Initialize(resourceManagers);
	sceneView.Initialize(renderDevice, inputManager);
	debugView.Initialize(debug);
}

void EditorCore::SetWorld(World* world)
{
	this->world = world;
}

void EditorCore::ResizeSceneViewFramebufferIfRequested()
{
	sceneView.ResizeFramebufferIfRequested();
}

const Framebuffer& EditorCore::GetSceneViewFramebuffer()
{
	return sceneView.GetFramebuffer();
}

SelectionContext& EditorCore::GetSelectionContext()
{
	return selectionContext;
}

CameraParameters EditorCore::GetEditorCameraParameters() const
{
	return sceneView.GetCameraParameters();
}

bool EditorCore::IsExitRequested() const
{
	return exitRequested;
}

void EditorCore::Update(EngineSettings* engineSettings)
{
	DrawMainMenuBar();

	sceneView.Update();

	entityListView.Draw(editorWindows[EditorWindow_Entities], selectionContext, world);
	entityView.Draw(editorWindows[EditorWindow_Properties], selectionContext, world);
	debugView.Draw(editorWindows[EditorWindow_Debug], engineSettings);
}

void EditorCore::DrawSceneView()
{
	sceneView.Draw(editorWindows[EditorWindow_Scene], world, selectionContext);
}

void EditorCore::EndFrame()
{
	for (size_t i = 0; i < EditorWindow_COUNT; ++i)
		editorWindows[i].requestFocus = false;
}

void EditorCore::DrawMainMenuBar()
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

			if (ImGui::MenuItem("Exit"))
				exitRequested;

			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Edit"))
		{
			if (ImGui::MenuItem("Copy"))
			{
				CopyEntity();
			}

			if (ImGui::MenuItem("Paste"))
			{
				PasteEntity();
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
		filePicker.StartDialogFileOpen("Open level", "Open");

	if (saveLevel)
		filePicker.StartDialogFileSave("Save level as", "Save");

	std::filesystem::path filePickerPathOut;
	bool filePickerClosed = filePicker.Update(filePickerPathOut);

	if (filePickerClosed && filePickerPathOut.empty() == false)
	{
		std::string pathStr = filePickerPathOut.u8string();
		std::string filenameStr = filePickerPathOut.filename().u8string();

		FilePickerDialog::DialogType type = filePicker.GetLastDialogType();
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

void EditorCore::CopyEntity()
{
	if (selectionContext.selectedEntity != Entity::Null)
	{
		LevelSerializer* serializer = world->GetSerializer();

		ArrayView<Entity> entities = ArrayView(&selectionContext.selectedEntity, 1);
		serializer->SerializeEntitiesToString(entities, copiedEntity);

		KK_LOG_DEBUG("Copied entity:\n{}", copiedEntity.GetCStr());
	}
}

void EditorCore::PasteEntity()
{
	if (copiedEntity.GetLength() > 0)
	{
		KK_LOG_DEBUG("Pasting entity:\n{}", copiedEntity.GetCStr());

		LevelSerializer* serializer = world->GetSerializer();

		SceneObjectId parent = SceneObjectId::Null;

		serializer->DeserializeEntitiesFromString(copiedEntity.GetCStr(), parent);
	}
}