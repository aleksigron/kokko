#include "EditorCore.hpp"

#include "imgui.h"

#include "Core/Core.hpp"

#include "Engine/Engine.hpp"
#include "Engine/World.hpp"

#include "Rendering/CameraParameters.hpp"

#include "Resources/LevelSerializer.hpp"

#include "System/Window.hpp"

#include "AssetBrowserView.hpp"
#include "DebugView.hpp"
#include "EntityListView.hpp"
#include "EntityView.hpp"
#include "SceneView.hpp"

namespace kokko
{
namespace editor
{

EditorCore::EditorCore(Allocator* allocator) :
	allocator(allocator),
	copiedEntity(allocator),
	editorWindows(allocator),
	sceneView(nullptr)
{
}

EditorCore::~EditorCore()
{
	for (EditorWindow* window : editorWindows)
		allocator->MakeDelete(window);
}

void EditorCore::Initialize(Engine* engine, const EditorProject* editorProject)
{
	editorContext.project = editorProject;
	editorContext.world = engine->GetWorld();
	editorContext.engineSettings = engine->GetSettings();

	images.LoadImages(engine->GetTextureManager());

	EntityListView* entityListView = allocator->MakeNew<EntityListView>();
	editorWindows.PushBack(entityListView);

	EntityView* entityView = allocator->MakeNew<EntityView>();
	entityView->Initialize(engine->GetMaterialManager(), engine->GetMeshManager());
	editorWindows.PushBack(entityView);

	sceneView = allocator->MakeNew<SceneView>();
	sceneView->Initialize(engine->GetRenderDevice(), engine->GetMainWindow());
	editorWindows.PushBack(sceneView);

	AssetBrowserView* assetBrowserView = allocator->MakeNew<AssetBrowserView>();
	assetBrowserView->Initialize(&images);
	editorWindows.PushBack(assetBrowserView);

	DebugView* debugView = allocator->MakeNew<DebugView>();
	debugView->Initialize(engine->GetDebug());
	editorWindows.PushBack(debugView);
}

void EditorCore::ResizeSceneViewFramebufferIfRequested()
{
	sceneView->ResizeFramebufferIfRequested();
}

const Framebuffer& EditorCore::GetSceneViewFramebuffer()
{
	return sceneView->GetFramebuffer();
}

CameraParameters EditorCore::GetEditorCameraParameters() const
{
	return sceneView->GetCameraParameters();
}

ArrayView<EditorWindow*> EditorCore::GetWindows()
{
	return editorWindows.GetView();
}

void EditorCore::NotifyProjectChanged()
{
	for (EditorWindow* window : editorWindows)
		window->OnEditorProjectChanged(editorContext);
}

void EditorCore::Update()
{
	for (EditorWindow* window : editorWindows)
		window->Update(editorContext);
}

void EditorCore::LateUpdate()
{
	for (EditorWindow* window : editorWindows)
		window->LateUpdate(editorContext);
}

void EditorCore::EndFrame()
{
	for (EditorWindow* window : editorWindows)
		window->requestFocus = false;
}

void EditorCore::CopyEntity()
{
	if (editorContext.selectedEntity != Entity::Null)
	{
		LevelSerializer* serializer = editorContext.world->GetSerializer();

		ArrayView<Entity> entities = ArrayView(&editorContext.selectedEntity, 1);
		serializer->SerializeEntitiesToString(entities, copiedEntity);

		KK_LOG_DEBUG("Copied entity:\n{}", copiedEntity.GetCStr());
	}
}

void EditorCore::PasteEntity()
{
	if (copiedEntity.GetLength() > 0)
	{
		KK_LOG_DEBUG("Pasting entity:\n{}", copiedEntity.GetCStr());

		LevelSerializer* serializer = editorContext.world->GetSerializer();

		SceneObjectId parent = SceneObjectId::Null;

		serializer->DeserializeEntitiesFromString(copiedEntity.GetCStr(), parent);
	}
}

}
}
