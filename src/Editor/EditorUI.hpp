#pragma once

#include "Editor/EditorCamera.hpp"

struct GLFWwindow;

class Allocator;
class InputView;

class Engine;
class EntityManager;
class World;
class Renderer;
class LightManager;
class CameraSystem;

class ImGuiRenderBackend;
class ImGuiPlatformBackend;

struct EditorViews;
struct Mat4x4fBijection;
struct ViewRectangle;

class EditorUI
{
private:
	Allocator* allocator;
	ImGuiRenderBackend* renderBackend;
	ImGuiPlatformBackend* platformBackend;

	EntityManager* entityManager;
	World* world;
	Renderer* renderer;
	LightManager* lightManager;
	CameraSystem* cameraSystem;

	EditorViews* views;

	EditorCamera editorCamera;

	void DrawMainMenuBar();

	void ClearAllEntities();

public:
	EditorUI(Allocator* allocator);
	EditorUI(const EditorUI&) = delete;
	EditorUI(EditorUI&&) = delete;
	~EditorUI();

	EditorUI& operator=(const EditorUI&) = delete;
	EditorUI& operator=(EditorUI&&) = delete;

	void Initialize(Engine* engine);
	void Deinitialize();

	void StartFrame();
	void Update();
	void EndFrame();

	ViewRectangle GetWorldViewport();

	Mat4x4fBijection GetEditorCameraTransform() const;
	ProjectionParameters GetEditorCameraProjection() const;
};
