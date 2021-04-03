#pragma once

struct GLFWwindow;

class ImGuiPlatformBackend
{
private:
	GLFWwindow* glfwWindow;

	void UpdateMousePosAndButtons();
	void UpdateMouseCursor();
	void UpdateGamepads();

	static void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
	static void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset);
	static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
	static void CharCallback(GLFWwindow* window, unsigned int c);

public:
	ImGuiPlatformBackend();
	ImGuiPlatformBackend(const ImGuiPlatformBackend&) = delete;
	ImGuiPlatformBackend(ImGuiPlatformBackend&&) = delete;
	~ImGuiPlatformBackend();

	ImGuiPlatformBackend& operator=(const ImGuiPlatformBackend&) = delete;
	ImGuiPlatformBackend& operator=(ImGuiPlatformBackend&&) = delete;

	bool Initialize(GLFWwindow* window);
	void Deinitialize();
	void NewFrame();
};
