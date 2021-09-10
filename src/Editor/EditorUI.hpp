#pragma once

#include <cstdint>

struct GLFWwindow;

class Allocator;
class Debug;
class EditorCore;
class Framebuffer;
class InputView;
class RenderDevice;
class Window;
class World;

class ImGuiRenderBackend;
class ImGuiPlatformBackend;

struct CameraParameters;
struct Mat4x4fBijection;
struct ViewRectangle;
struct ResourceManagers;

class EditorUI
{
private:
	Allocator* allocator;
	RenderDevice* renderDevice;
	ImGuiRenderBackend* renderBackend;
	ImGuiPlatformBackend* platformBackend;

	EditorCore* core;

public:
	EditorUI(Allocator* allocator);
	EditorUI(const EditorUI&) = delete;
	EditorUI(EditorUI&&) = delete;
	~EditorUI();

	EditorUI& operator=(const EditorUI&) = delete;
	EditorUI& operator=(EditorUI&&) = delete;

	void Initialize(Debug* debug, RenderDevice* renderDevice, Window* window,
		const ResourceManagers& resourceManagers);
	void Deinitialize();

	void StartFrame();
	void Update(World* world, bool& shouldExitOut);
	void DrawSceneView(World* world);
	void EndFrame();

	const Framebuffer& GetSceneViewFramebuffer();

	CameraParameters GetEditorCameraParameters() const;
};
