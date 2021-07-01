#pragma once


struct GLFWwindow;

class Allocator;
class InputView;
class RenderDevice;
class Framebuffer;
class Window;
class World;

class ImGuiRenderBackend;
class ImGuiPlatformBackend;

struct CameraParameters;
struct EditorViews;
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

	EditorViews* views;

	void Draw(World* world, bool& shouldExitOut);
	void DrawMainMenuBar(World* world, bool& shouldExitOut);

public:
	EditorUI(Allocator* allocator);
	EditorUI(const EditorUI&) = delete;
	EditorUI(EditorUI&&) = delete;
	~EditorUI();

	EditorUI& operator=(const EditorUI&) = delete;
	EditorUI& operator=(EditorUI&&) = delete;

	void Initialize(RenderDevice* renderDevice, Window* window,
		const ResourceManagers& resourceManagers);
	void Deinitialize();

	void StartFrame();
	void Update(World* world, bool& shouldExitOut);
	void DrawSceneView();
	void EndFrame();

	const Framebuffer& GetSceneViewFramebuffer();

	CameraParameters GetEditorCameraParameters() const;
};
