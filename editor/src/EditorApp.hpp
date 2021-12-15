#pragma once

#include <filesystem>

#include "EditorProject.hpp"
#include "FilePickerDialog.hpp"

class Allocator;
class Engine;
class FilesystemVirtual;
class Framebuffer;
class RenderDevice;
class World;

struct CameraParameters;
struct EngineSettings;

namespace kokko
{
namespace editor
{

class AssetLibrary;
class EditorCore;

enum class EditorWindowType;

class EditorApp
{
public:
	EditorApp(Allocator* allocator, FilesystemVirtual* filesystem);
	EditorApp(const EditorApp&) = delete;
	EditorApp(EditorApp&&) = delete;
	~EditorApp();

	EditorApp& operator=(const EditorApp&) = delete;
	EditorApp& operator=(EditorApp&&) = delete;

	void Initialize(Engine* engine);
	void Deinitialize();

	void StartFrame();
	void Update(EngineSettings* engineSettings, bool& shouldExitOut);
	void EndFrame();

	const Framebuffer& GetSceneViewFramebuffer();

	AssetLibrary* GetAssetLibrary();

	CameraParameters GetEditorCameraParameters() const;

private:
	enum class MainMenuDialog
	{
		None,
		CreateProject,
		OpenProject,
		OpenLevel,
		SaveLevelAs
	};

	void DrawMainMenuBar();
	void ResetMainMenuDialog();

	bool CreateProject(const std::filesystem::path& directory, StringRef name);
	bool OpenProject(const std::filesystem::path& projectDir);
	void OnProjectChanged();

	Engine* engine;
	FilesystemVirtual* virtualFilesystem;
	Allocator* allocator;
	RenderDevice* renderDevice;
	World* world;

	EditorCore* core;
	FilePickerDialog filePicker;

	EditorProject project;

	bool exitRequested;

	MainMenuDialog currentMainMenuDialog;
	uint32_t currentDialogId;
};

}
}
