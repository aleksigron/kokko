#pragma once

#include <filesystem>

#include "Core/Array.hpp"
#include "Core/Optional.hpp"
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

struct Uid;

namespace editor
{

class EditorProject;
class EditorWindow;
class SceneView;

class EditorCore
{
public:
	EditorCore(Allocator* allocator, Filesystem* filesystem, FilesystemResolver* resolver);
	~EditorCore();

	void Initialize(Engine* engine);
	void Deinitialize();

	void ResizeSceneViewFramebufferIfRequested();
	const Framebuffer& GetSceneViewFramebuffer();
	CameraParameters GetEditorCameraParameters() const;

	ArrayView<EditorWindow*> GetWindows();

	AssetLibrary* GetAssetLibrary();
	Optional<Uid> GetLoadedLevelUid() const;

	void NotifyProjectChanged(const EditorProject* editorProject);

	void Update();
	void LateUpdate();
	void EndFrame();

	void NewLevel();
	void OpenLevel(Uid levelAssetUid);
	void SaveLevel();
	void SaveLevelAs(const std::filesystem::path& pathRelativeToAssets);

	void CopyEntity();
	void PasteEntity();

private:
	Allocator* allocator;
	Filesystem* filesystem;
	EditorContext editorContext;
	EditorImages images;

	AssetLibrary assetLibrary;

	String copiedEntity;

	Array<EditorWindow*> editorWindows;

	SceneView* sceneView;
};

}
}
