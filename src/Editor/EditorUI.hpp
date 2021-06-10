#pragma once

#include "Editor/EditorCamera.hpp"

struct GLFWwindow;

class Allocator;
class InputView;

class Window;
class World;

class ImGuiRenderBackend;
class ImGuiPlatformBackend;

struct EditorViews;
struct Mat4x4fBijection;
struct ViewRectangle;
struct ResourceManagers;

class EditorUI
{
private:
	Allocator* allocator;
	ImGuiRenderBackend* renderBackend;
	ImGuiPlatformBackend* platformBackend;

	EditorViews* views;

	EditorCamera editorCamera;

	void DrawMainMenuBar(World* world);

	void ClearAllEntities(World* world);

public:
	EditorUI(Allocator* allocator);
	EditorUI(const EditorUI&) = delete;
	EditorUI(EditorUI&&) = delete;
	~EditorUI();

	EditorUI& operator=(const EditorUI&) = delete;
	EditorUI& operator=(EditorUI&&) = delete;

	void Initialize(Window* window, const ResourceManagers& resourceManagers);
	void Deinitialize();

	void StartFrame();
	void Update(World* world);
	void EndFrame();

	ViewRectangle GetWorldViewport();

	Mat4x4fBijection GetEditorCameraTransform() const;
	ProjectionParameters GetEditorCameraProjection() const;
};
