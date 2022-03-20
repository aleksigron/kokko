#pragma once

#include <filesystem>

#include "Math/Vec2.hpp"

#include "EditorProject.hpp"
#include "EditorUserSettings.hpp"
#include "FilePickerDialog.hpp"

class Allocator;
class Engine;
class Framebuffer;
class RenderDevice;
class World;
class Window;

struct CameraParameters;
struct EngineSettings;

namespace kokko
{

class Filesystem;
class FilesystemResolverVirtual;

struct Uid;

namespace editor
{

class AssetLibrary;
class EditorCore;

enum class EditorWindowType;

class EditorApp
{
public:
	EditorApp(Allocator* allocator, Filesystem* filesystem, FilesystemResolverVirtual* resolver);
	EditorApp(const EditorApp&) = delete;
	EditorApp(EditorApp&&) = delete;
	~EditorApp();

	EditorApp& operator=(const EditorApp&) = delete;
	EditorApp& operator=(EditorApp&&) = delete;

	void LoadUserSettings();
	const EditorUserSettings& GetUserSettings() const;

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
		SaveLevelAs
	};

	void DrawMainMenuBar();
	void ResetMainMenuDialog();

	bool CreateProject(const std::filesystem::path& directory, StringRef name);
	bool OpenProject(const std::filesystem::path& projectDir);
	void OnProjectChanged();

	static void OnWindowResize(void* app, Window* window, Vec2i size);
	static void OnWindowMaximize(void* app, Window* window, bool maximized);

	Engine* engine;
	FilesystemResolverVirtual* filesystemResolver;
	Allocator* allocator;
	RenderDevice* renderDevice;
	World* world;

	EditorCore* core;
	FilePickerDialog filePicker;

	EditorProject project;
	EditorUserSettings userSettings;

	bool exitRequested;

	MainMenuDialog currentMainMenuDialog;
	uint64_t currentDialogId;
};

}
}
