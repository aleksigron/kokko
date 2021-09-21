#pragma once

class Allocator;
class Engine;
class World;

struct EngineSettings;

#include "Core/String.hpp"

#include "AssetBrowserView.hpp"
#include "DebugView.hpp"
#include "EditorWindowInfo.hpp"
#include "EntityListView.hpp"
#include "EntityView.hpp"
#include "FilePickerDialog.hpp"
#include "SceneView.hpp"

#include "EditorImages.hpp"
#include "SelectionContext.hpp"

class EditorCore
{
public:
	EditorCore(Allocator* allocator);

	void Initialize(Engine* engine);

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
	EditorImages images;

	FilePickerDialog filePicker;

	EntityListView entityListView;
	EntityView entityView;
	SceneView sceneView;
	AssetBrowserView assetBrowserView;
	DebugView debugView;

	String copiedEntity;
};
