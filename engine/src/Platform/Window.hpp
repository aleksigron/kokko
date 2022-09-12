#pragma once

#include "Core/Array.hpp"
#include "Core/Pair.hpp"

#include "Math/Vec2.hpp"

class Allocator;
class InputManager;

struct Mat4x4f;
struct GLFWwindow;

namespace kokko
{
class NativeRenderDevice;

struct WindowSettings;

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
    virtual ~Window();

    // Implemented by the specific render backend window type
    virtual GLFWwindow* CreateWindow(const kokko::WindowSettings& settings, NativeRenderDevice* device) = 0;

    bool Initialize(const kokko::WindowSettings& settings, NativeRenderDevice* device);

    void UpdateInput();
    void ProcessEvents();
    void Swap();

    bool GetShouldClose();
    void SetShouldClose(bool shouldClose);
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

    void SetWindowTitle(const char* title);

    void SetCursorMode(CursorMode mode);
    CursorMode GetCursorMode() const;

    InputManager* GetInputManager() { return inputManager; }

    GLFWwindow* GetGlfwWindow() { return windowHandle; }
    void SetGlfwWindow(GLFWwindow* window) { windowHandle = window; }

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


} // namespace kokko

