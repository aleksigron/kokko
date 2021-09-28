#pragma once

#include "EditorProject.hpp"

class Allocator;
class EditorCore;
class Engine;
class Framebuffer;
class RenderDevice;
class World;

struct CameraParameters;
struct EngineSettings;

class EditorApp
{
private:
	Allocator* allocator;
	RenderDevice* renderDevice;

	EditorCore* core;

	EditorProject project;

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
};
