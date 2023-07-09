#pragma once

#include <filesystem>

#include "Core/UniquePtr.hpp"

#include "Math/Vec2.hpp"

#include "EditorProject.hpp"
#include "EditorUserSettings.hpp"
#include "FilePickerDialog.hpp"

class Allocator;
class Engine;

struct CameraParameters;

namespace kokko
{

class AssetLibrary;
class Filesystem;
class FilesystemResolverVirtual;
class ImguiImplOpenGL;
class Window;
class World;

struct Uid;
struct EngineSettings;

namespace render
{
class CommandEncoder;
class Device;
class Framebuffer;
}

namespace editor
{

class ConsoleLogger;
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

	void Initialize(Engine* engine, ConsoleLogger* consoleLogger);
	void Deinitialize();

	void StartFrame();
	void Update(kokko::EngineSettings* engineSettings, bool& shouldExitOut);
	void EndFrame(render::CommandEncoder* encoder);

	const render::Framebuffer& GetSceneViewFramebuffer();

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

	bool CreateProject(const std::filesystem::path& directory, ConstStringView name);
	bool OpenProject(const std::filesystem::path& projectDir);
	void OnProjectChanged();

	static void OnWindowResize(void* app, Window* window, Vec2i size);
	static void OnWindowMaximize(void* app, Window* window, bool maximized);

	Engine* engine;
	FilesystemResolverVirtual* filesystemResolver;
	Allocator* allocator;
	kokko::render::Device* renderDevice;
	World* world;

	UniquePtr<ImguiImplOpenGL> imguiImplOpenGL;
	UniquePtr<EditorCore> core;
	FilePickerDialog filePicker;

	EditorProject project;
	EditorUserSettings userSettings;

	bool exitRequested;

	MainMenuDialog currentMainMenuDialog;
	uint64_t currentDialogId;
};

} // namespace editor
} // namespace kokko
