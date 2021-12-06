#pragma once

#include "Core/Array.hpp"
#include "Core/String.hpp"

#include "FilePickerDialog.hpp"

#include "AssetLibrary.hpp"
#include "EditorImages.hpp"
#include "EditorContext.hpp"

class Allocator;
class Engine;
class Filesystem;
class World;
class Framebuffer;

struct CameraParameters;
struct EngineSettings;

namespace kokko
{
namespace editor
{

class EditorProject;
class EditorWindow;
class SceneView;

class EditorCore
{
public:
	EditorCore(Allocator* allocator, Filesystem* filesystem);
	~EditorCore();

	void Initialize(Engine* engine);

	void ResizeSceneViewFramebufferIfRequested();
	const Framebuffer& GetSceneViewFramebuffer();
	CameraParameters GetEditorCameraParameters() const;

	ArrayView<EditorWindow*> GetWindows();

	void NotifyProjectChanged(const EditorProject* editorProject);

	void Update();
	void LateUpdate();
	void EndFrame();

	void CopyEntity();
	void PasteEntity();

private:
	Allocator* allocator;
	EditorContext editorContext;
	EditorImages images;

	AssetLibrary assetLibrary;

	String copiedEntity;

	Array<EditorWindow*> editorWindows;

	SceneView* sceneView;
};

}
}
