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

private:
	Allocator* allocator;
    Window* window;
    bool glfwInitialized;
};

} // namespace kokko

