#pragma once

struct GLFWwindow;

class Allocator;
class InputView;
class Engine;

class ImGuiRenderBackend;
class ImGuiPlatformBackend;

struct EditorViews;

struct ViewRectangle;

class EditorUI
{
private:
	Allocator* allocator;
	ImGuiRenderBackend* renderBackend;
	ImGuiPlatformBackend* platformBackend;

	EditorViews* views;

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
	void Render();

	ViewRectangle GetWorldViewport();
};
