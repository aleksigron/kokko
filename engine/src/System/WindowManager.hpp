#pragma once

#include "Core/Array.hpp"
#include "Core/Pair.hpp"

#include "Math/Vec2.hpp"
#include "Math/Mat4x4.hpp"

class Allocator;
class InputManager;

namespace kokko
{
class NativeRenderDevice;
class Window;
struct WindowSettings;

class WindowManager
{
public:
	WindowManager(Allocator* allocator);
	~WindowManager();
	
	bool Initialize(const kokko::WindowSettings& settings, NativeRenderDevice* device);

	void ProcessEvents();

    Window* GetWindow();

	/*
	Set the number of screen refresh to wait for until swapping buffers.
	0: vsync off, 1: vsync every refresh, n: vsync once every n refreshes
	*/
	void SetSwapInterval(int swapInterval);
	int GetSwapInterval() const;

private:
	Allocator* allocator;
    Window* window;
    int currentSwapInterval;
    bool glfwInitialized;
};

} // namespace kokko

