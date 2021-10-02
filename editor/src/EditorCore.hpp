#pragma once

class Allocator;
class Engine;
class World;
class Framebuffer;

class EditorProject;
class EditorWindow;
class SceneView;

struct CameraParameters;
struct EngineSettings;

#include "Core/Array.hpp"
#include "Core/String.hpp"

#include "FilePickerDialog.hpp"

#include "EditorImages.hpp"
#include "EditorContext.hpp"

class EditorCore
{
public:
	EditorCore(Allocator* allocator);
	~EditorCore();

	void Initialize(Engine* engine, const EditorProject* editorProject);

	void ResizeSceneViewFramebufferIfRequested();
	const Framebuffer& GetSceneViewFramebuffer();
	CameraParameters GetEditorCameraParameters() const;

	ArrayView<EditorWindow*> GetWindows();

	void NotifyProjectChanged();

	void Update();
	void LateUpdate();
	void EndFrame();

	void CopyEntity();
	void PasteEntity();

private:
	Allocator* allocator;
	EditorContext editorContext;
	EditorImages images;

	String copiedEntity;

	Array<EditorWindow*> editorWindows;

	SceneView* sceneView;
};
