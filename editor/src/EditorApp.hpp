#pragma once

#include "EditorProject.hpp"
#include "FilePickerDialog.hpp"

class Allocator;
class EditorCore;
class Engine;
class Framebuffer;
class RenderDevice;
class World;

class EditorWindow;

struct CameraParameters;
struct EngineSettings;

enum class EditorWindowType;

class EditorApp
{
public:
	EditorApp(Allocator* allocator);
	EditorApp(const EditorApp&) = delete;
	EditorApp(EditorApp&&) = delete;
	~EditorApp();

	EditorApp& operator=(const EditorApp&) = delete;
	EditorApp& operator=(EditorApp&&) = delete;

	void Initialize(Engine* engine);
	void Deinitialize();

	void StartFrame();
	void Update(EngineSettings* engineSettings, World* world, bool& shouldExitOut);
	void EndFrame();

	const Framebuffer& GetSceneViewFramebuffer();

	CameraParameters GetEditorCameraParameters() const;

private:
	void DrawMainMenuBar();

	Allocator* allocator;
	RenderDevice* renderDevice;

	EditorCore* core;
	FilePickerDialog filePicker;

	EditorProject project;

	World* world;

	bool exitRequested;
};
