#pragma once

#include "Core/Array.hpp"
#include "Core/Pair.hpp"
#include "Core/UniquePtr.hpp"

#include "Math/Vec2.hpp"

struct GLFWwindow;

namespace kokko
{

class Allocator;
class InputManager;
class NativeRenderDevice;
class NativeSurface;

struct Mat4x4f;
struct TextureHandle;

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

    static Window* Create(Allocator* allocator);

    // Implemented by the specific render backend window type
    virtual GLFWwindow* CreateWindow(const kokko::WindowSettings& settings, NativeRenderDevice* device) = 0;

    virtual NativeSurface* GetNativeSurface();
    virtual TextureHandle GetNativeSurfaceTexture();
    virtual void ReleaseNativeSurface();

    bool Initialize(const kokko::WindowSettings& settings, NativeRenderDevice* device);

    void UpdateInput();
    void ProcessEvents();

    /*
    Set the number of screen refresh to wait for until swapping buffers.
    0: vsync off, 1: vsync every refresh, n: vsync once every n refreshes
    */
    virtual void SetSwapInterval(int swapInterval) {}
    virtual int GetSwapInterval() const { return -1; }
    virtual void Swap() {}

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

    InputManager* GetInputManager() { return inputManager.Get(); }

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
    UniquePtr<InputManager> inputManager;

    Array<Pair<ResizeCallbackFn, void*>> framebufferResizeCallbacks;
    Array<Pair<ResizeCallbackFn, void*>> windowResizeCallbacks;
    Array<Pair<ToggleCallbackFn, void*>> maximizeCallbacks;

    Vec2i currentFramebufferSize;
    Vec2i currentWindowSize;
    int currentSwapInterval;
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

