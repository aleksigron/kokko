#pragma once

class Allocator;
class World;

struct EngineSettings;

#include "Core/String.hpp"

#include "Editor/AssetBrowserView.hpp"
#include "Editor/DebugView.hpp"
#include "Editor/EditorWindowInfo.hpp"
#include "Editor/EntityListView.hpp"
#include "Editor/EntityView.hpp"
#include "Editor/FilePickerDialog.hpp"
#include "Editor/SceneView.hpp"
#include "Editor/SelectionContext.hpp"

class EditorCore
{
public:
	EditorCore(Allocator* allocator);

	void Initialize(Debug* debug, RenderDevice* renderDevice,
		InputManager* inputManager, const ResourceManagers& resourceManagers);

	void SetWorld(World* world);

	void ResizeSceneViewFramebufferIfRequested();
	const Framebuffer& GetSceneViewFramebuffer();
	SelectionContext& GetSelectionContext();
	CameraParameters GetEditorCameraParameters() const;

	bool IsExitRequested() const;

	void Update(EngineSettings* engineSettings);
	void DrawSceneView();
	void EndFrame();

private:
	void DrawMainMenuBar();

	void CopyEntity();
	void PasteEntity();

	enum EditorWindow
	{
		EditorWindow_Entities,
		EditorWindow_Properties,
		EditorWindow_Scene,
		EditorWindow_AssetBrowser,
		EditorWindow_Debug,

		EditorWindow_COUNT
	};

	EditorWindowInfo editorWindows[EditorWindow_COUNT];

	bool exitRequested;

	World* world;
	SelectionContext selectionContext;

	FilePickerDialog filePicker;

	EntityListView entityListView;
	EntityView entityView;
	SceneView sceneView;
	AssetBrowserView assetBrowserView;
	DebugView debugView;

	String copiedEntity;
};
