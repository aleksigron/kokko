#pragma once

#include "Core/Array.hpp"
#include "Core/Pair.hpp"

#include "Math/Vec2.hpp"
#include "Math/Mat4x4.hpp"

struct GLFWwindow;

class Allocator;
class InputManager;

namespace kokko
{
struct WindowSettings;
}

class Window
{
public:
	enum class CursorMode
	{
		Normal,
		Hidden,
		Disabled
	};

	using ResizeCallbackFn = void(*)(void*, Window*, Vec2i);
	using ToggleCallbackFn = void(*)(void*, Window*, bool);

	Window(Allocator* allocator);
	~Window();
	
	bool Initialize(const kokko::WindowSettings& settings);

	bool GetShouldClose();
	void SetShouldClose(bool shouldClose);

	void UpdateInput();
	void ProcessEvents();
	void Swap();
	
	/*
	Get framebuffer size in pixels
	*/
	Vec2i GetFrameBufferSize();

	/*
	Get window content area in screen coordinates
	*/
	Vec2i GetWindowSize();

	/*
	Get screen coordinate scale compared to pixels
	*/
	float GetScreenCoordinateScale();

	/*
	Get orthographic projection matrix for screen coordinate space
	*/
	Mat4x4f GetScreenSpaceProjectionMatrix();

	/*
	Set the number of screen refresh to wait for until swapping buffers.
	0: vsync off, 1: vsync every refresh, n: vsync once every n refreshes
	*/
	void SetSwapInterval(int swapInterval);
	int GetSwapInterval() const;

	void SetWindowTitle(const char* title);

	void SetCursorMode(CursorMode mode);
	CursorMode GetCursorMode() const;

	InputManager* GetInputManager() { return inputManager; }
	GLFWwindow* GetGlfwWindow() { return windowHandle; }

	void RegisterFramebufferResizeCallback(ResizeCallbackFn callback, void* userPointer);
	void UnregisterFramebufferResizeCallback(ResizeCallbackFn callback, void* userPointer);

	void RegisterWindowResizeCallback(ResizeCallbackFn callback, void* userPointer);
	void UnregisterWindowResizeCallback(ResizeCallbackFn callback, void* userPointer);

	void RegisterMaximizeCallback(ToggleCallbackFn callback, void* userPointer);
	void UnregisterMaximizeCallback(ToggleCallbackFn callback, void* userPointer);
	
	static Window* GetWindowObject(GLFWwindow* windowHandle);

private:
	Allocator* allocator;
	GLFWwindow* windowHandle;

	InputManager* inputManager;

	Array<Pair<ResizeCallbackFn, void*>> framebufferResizeCallbacks;
	Array<Pair<ResizeCallbackFn, void*>> windowResizeCallbacks;
	Array<Pair<ToggleCallbackFn, void*>> maximizeCallbacks;

	int currentSwapInterval;
	Vec2i currentFramebufferSize;
	Vec2i currentWindowSize;
	bool currentMaximizeState;

	bool framebufferResizePending;
	bool windowResizePending;
	bool maximizeChangePending;

	template <typename CallbackType>
	void UnregisterCallback(Array<Pair<CallbackType, void*>>& arr, CallbackType callback, void* userPtr);

	static void _GlfwFramebufferSizeCallback(GLFWwindow* window, int width, int height);
	static void _GlfwWindowSizeCallback(GLFWwindow* window, int width, int height);
	static void _GlfwMaximizeCallback(GLFWwindow* window, int maximized);
	
	void GlfwFramebufferSizeCallback(int width, int height);
	void GlfwWindowSizeCallback(int width, int height);
	void GlfwMaximizeCallback(int maximized);
};
