#pragma once

struct GLFWwindow;

class Allocator;
class InputView;
class EntityManager;

class ImGuiRenderBackend;
class ImGuiPlatformBackend;

class EditorViews;

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

	void Initialize(GLFWwindow* window, InputView* imguiInputView);
	void Deinitialize();

	void StartFrame();
	void Render(EntityManager* entityManager);
};
