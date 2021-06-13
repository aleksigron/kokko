#pragma once

#include "Core/Array.hpp"

#include "Math/Vec2.hpp"
#include "Math/Mat4x4.hpp"

struct GLFWwindow;

class Allocator;
class InputManager;

class Window
{
public:
	using FramebufferSizeCallbackFn = void(*)(void*, Window*, Vec2i);

	Window(Allocator* allocator);
	~Window();
	
	bool Initialize(int width, int height, const char* windowTitle);
	bool ShouldClose();

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
	int GetSwapInterval() const { return currentSwapInterval; }

	InputManager* GetInputManager() { return inputManager; }
	GLFWwindow* GetGlfwWindow() { return windowHandle; }

	void RegisterFramebufferResizeCallback(FramebufferSizeCallbackFn callback, void* userPointer);
	void UnregisterFramebufferResizeCallback(FramebufferSizeCallbackFn callback, void* userPointer);
	
	static Window* GetWindowObject(GLFWwindow* windowHandle);

private:
	struct FramebufferResizeCallbackInfo
	{
		FramebufferSizeCallbackFn function;
		void* userPointer;
	};

	Allocator* allocator;
	GLFWwindow* windowHandle;

	InputManager* inputManager;

	Array<FramebufferResizeCallbackInfo> framebufferResizeCallbacks;

	int currentSwapInterval;
	Vec2i currentFramebufferSize;

	bool framebufferResizePending;

	static void _GlfwFramebufferSizeCallback(GLFWwindow* window, int width, int height);
	void GlfwFramebufferSizeCallback(int width, int height);
};
