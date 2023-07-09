#include "EditorCore.hpp"

#include "imgui.h"

#include "Core/Core.hpp"

#include "Engine/Engine.hpp"
#include "Engine/EngineConstants.hpp"
#include "Engine/World.hpp"

#include "Platform/Window.hpp"

#include "Rendering/CameraParameters.hpp"

#include "Resources/LevelSerializer.hpp"

#include "System/Filesystem.hpp"
#include "System/WindowManager.hpp"

#include "App/EditorConstants.hpp"
#include "App/EditorProject.hpp"

#include "System/ConsoleLogger.hpp"

#include "Views/AssetBrowserView.hpp"
#include "Views/AssetView.hpp"
#include "Views/ConsoleView.hpp"
#include "Views/DebugView.hpp"
#include "Views/EntityListView.hpp"
#include "Views/EntityView.hpp"
#include "Views/SceneView.hpp"

namespace kokko
{
namespace editor
{

EditorCore::EditorCore(Allocator* allocator, Filesystem* filesystem, FilesystemResolver* resolver) :
	allocator(allocator),
	filesystem(filesystem),
	editorContext(allocator, resolver),
	assetLibrary(allocator, filesystem),
	copiedEntity(allocator),
	editorWindows(allocator),
	sceneView(nullptr)
{
	auto appConfig = AssetScopeConfiguration{
		EditorConstants::EditorResourcePath,
		String(allocator, EditorConstants::VirtualMountEditor)
	};

	assetLibrary.SetAppScopeConfig(appConfig);
}

EditorCore::~EditorCore()
{
	for (EditorWindow* window : editorWindows)
		allocator->MakeDelete(window);
}

void EditorCore::Initialize(Engine* engine, ConsoleLogger* consoleLogger)
{
	editorContext.project = nullptr;
	editorContext.world = engine->GetWorld();
	editorContext.engineSettings = engine->GetSettings();
	editorContext.monospaceFont = ImGui::GetIO().Fonts->Fonts[1];

	images.LoadImages(engine->GetTextureManager());

	EntityListView* entityListView = allocator->MakeNew<EntityListView>();
	editorWindows.PushBack(entityListView);

	EntityView* entityView = allocator->MakeNew<EntityView>();
	entityView->Initialize(engine->GetResourceManagers());
	editorWindows.PushBack(entityView);

	sceneView = allocator->MakeNew<SceneView>();
	sceneView->Initialize(engine->GetRenderDevice(), engine->GetWindowManager()->GetWindow());
	editorWindows.PushBack(sceneView);

	AssetBrowserView* assetBrowserView = allocator->MakeNew<AssetBrowserView>(allocator);
	assetBrowserView->Initialize(&images);
	editorWindows.PushBack(assetBrowserView);

	AssetView* assetView = allocator->MakeNew<AssetView>(allocator);
	assetView->Initialize(engine->GetMaterialManager(), engine->GetShaderManager(), engine->GetTextureManager());
	editorWindows.PushBack(assetView);

	DebugView* debugView = allocator->MakeNew<DebugView>();
	debugView->Initialize(engine->GetDebug());
	editorWindows.PushBack(debugView);

	ConsoleView* consoleView = allocator->MakeNew<ConsoleView>(allocator);
	editorWindows.PushBack(consoleView);
	consoleLogger->SetConsoleView(consoleView);
}

void EditorCore::Deinitialize()
{
	for (EditorWindow* window : editorWindows)
		window->ReleaseEngineResources();
}

void EditorCore::ResizeSceneViewFramebufferIfRequested()
{
	sceneView->ResizeFramebufferIfRequested();
}

const render::Framebuffer& EditorCore::GetSceneViewFramebuffer()
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

AssetLibrary* EditorCore::GetAssetLibrary()
{
	return &assetLibrary;
}

Optional<Uid> EditorCore::GetLoadedLevelUid() const
{
	return editorContext.loadedLevel;
}

void EditorCore::NotifyProjectChanged(const EditorProject* editorProject)
{
	auto projectConfig = AssetScopeConfiguration{
		editorProject->GetAssetPath(),
		String(allocator, EngineConstants::VirtualMountAssets)
	};

	assetLibrary.SetProjectScopeConfig(projectConfig);
	assetLibrary.ScanAssets(false, false, true);

	editorContext.project = editorProject;
	editorContext.assetLibrary = &assetLibrary;

	for (EditorWindow* window : editorWindows)
		window->OnEditorProjectChanged(editorContext);
}

void EditorCore::Update()
{
	if (editorContext.requestLoadLevel.HasValue())
	{
		if (editorContext.loadedLevel.HasValue() == false ||
			editorContext.requestLoadLevel.GetValue() != editorContext.loadedLevel.GetValue())
		{
			OpenLevel(editorContext.requestLoadLevel.GetValue());
		}

		editorContext.requestLoadLevel = Optional<Uid>();
	}

	for (EditorWindow* window : editorWindows)
		window->Update(editorContext);

	editorContext.engineSettings->renderDebug.SetDebugEntity(editorContext.selectedEntity);
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

void EditorCore::NewLevel()
{
	editorContext.world->ClearAllEntities();
	editorContext.loadedLevel = Optional<Uid>();
}

void EditorCore::OpenLevel(Uid levelAssetUid)
{
	KOKKO_PROFILE_FUNCTION();

	if (auto asset = editorContext.assetLibrary->FindAssetByUid(levelAssetUid))
	{
		String sceneConfig(allocator);

		if (filesystem->ReadText(asset->GetVirtualPath().GetCStr(), sceneConfig))
		{
			editorContext.world->GetSerializer()->DeserializeFromString(sceneConfig.GetData());

			editorContext.loadedLevel = levelAssetUid;
			return;
		}
	}

	KK_LOG_ERROR("EditorCore: Couldn't load level");
}

void EditorCore::SaveLevel()
{
	if (editorContext.loadedLevel.HasValue() == false)
		return;

	auto asset = editorContext.assetLibrary->FindAssetByUid(editorContext.loadedLevel.GetValue());

	if (asset == nullptr)
		return;

	String content(allocator);
	editorContext.world->GetSerializer()->SerializeToString(content);
	ArrayView<const uint8_t> contentView(reinterpret_cast<const uint8_t*>(content.GetData()), content.GetLength());

	if (editorContext.assetLibrary->UpdateAssetContent(asset->GetUid(), contentView) == false)
	{
		KK_LOG_ERROR("EditorCore: Failed to update level asset content");
	}
}

void EditorCore::SaveLevelAs(const std::filesystem::path& pathRelativeToAssets)
{
	String content(allocator);
	editorContext.world->GetSerializer()->SerializeToString(content);
	ArrayView<const uint8_t> contentView(reinterpret_cast<const uint8_t*>(content.GetData()), content.GetLength());

	// TODO: Extract to a function
	std::string pathStdStr = EngineConstants::VirtualMountAssets + ('/' + pathRelativeToAssets.generic_u8string());
	String pathStr(allocator);
	pathStr.Assign(ConstStringView(pathStdStr.c_str(), pathStdStr.length()));

	auto asset = editorContext.assetLibrary->FindAssetByVirtualPath(pathStr);
	Optional<Uid> assetUid;

	if (asset != nullptr)
	{
		if (asset->GetType() != AssetType::Level)
		{
			KK_LOG_ERROR("EditorCore: Couldn't overwrite asset {} because it is not a level asset.",
				asset->GetVirtualPath().GetCStr());
			return;
		}

		assetUid = asset->GetUid();

		// Update asset
		if (editorContext.assetLibrary->UpdateAssetContent(asset->GetUid(), contentView) == false)
		{
			KK_LOG_ERROR("EditorCore: Failed to update level asset content");
		}
	}
	else
	{
		std::string relativePath = pathRelativeToAssets.generic_u8string();
		ConstStringView relativePathRef(relativePath.c_str(), relativePath.length());

		// Create asset
		assetUid = editorContext.assetLibrary->CreateAsset(AssetType::Level, relativePathRef, contentView);

		if (assetUid.HasValue() == false)
			KK_LOG_ERROR("EditorCore: Failed to create new level asset");
	}

	if (assetUid.HasValue())
		editorContext.loadedLevel = assetUid.GetValue();
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
