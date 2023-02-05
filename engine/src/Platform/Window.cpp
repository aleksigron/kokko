#include "Platform/Window.hpp"

#include "Math/Mat4x4.hpp"

#ifdef KOKKO_USE_METAL
#include "Platform/WindowMetal.hpp"
#endif

#ifdef KOKKO_USE_OPENGL
#include "Platform/WindowOpenGL.hpp"
#endif

#include "Rendering/RenderTypes.hpp"

#include "System/IncludeGLFW.hpp"
#include "System/InputManager.hpp"

namespace kokko
{

Window::Window(Allocator* allocator) :
    allocator(allocator),
    windowHandle(nullptr),
    inputManager(nullptr),
    framebufferResizeCallbacks(allocator),
    windowResizeCallbacks(allocator),
    maximizeCallbacks(allocator),
    currentSwapInterval(-1),
    currentMaximizeState(false),
    framebufferResizePending(false),
    windowResizePending(false),
    maximizeChangePending(false)
{

}

Window::~Window()
{
    allocator->MakeDelete(inputManager);

    if (windowHandle != nullptr)
    {
        glfwDestroyWindow(windowHandle);
    }
}

Window* Window::Create(Allocator* allocator)
{
#ifdef KOKKO_USE_METAL
    return allocator->MakeNew<WindowMetal>(allocator);
#else
    return allocator->MakeNew<WindowOpenGL>(allocator);
#endif
}

NativeSurface* Window::GetNativeSurface()
{
    return nullptr;
}

TextureHandle Window::GetNativeSurfaceTexture()
{
    return TextureHandle{};
}

void Window::ReleaseNativeSurface() {}

bool Window::Initialize(const WindowSettings& settings, NativeRenderDevice* device)
{
    windowHandle = CreateWindow(settings, device);

    if (windowHandle != nullptr)
    {
        glfwGetFramebufferSize(windowHandle, &currentFramebufferSize.x, &currentFramebufferSize.y);
        glfwGetWindowSize(windowHandle, &currentWindowSize.x, &currentWindowSize.y);
        int maximized = glfwGetWindowAttrib(windowHandle, GLFW_MAXIMIZED);
        currentMaximizeState = maximized == GLFW_TRUE;

        inputManager = allocator->MakeNew<InputManager>(allocator);
        inputManager->Initialize(windowHandle);

        glfwSetWindowUserPointer(windowHandle, this);

        glfwSetFramebufferSizeCallback(windowHandle, _GlfwFramebufferSizeCallback);
        glfwSetWindowSizeCallback(windowHandle, _GlfwWindowSizeCallback);
        glfwSetWindowMaximizeCallback(windowHandle, _GlfwMaximizeCallback);

        return true;
    }

    return false;
}

void Window::UpdateInput()
{
    inputManager->Update();
}

void Window::ProcessEvents()
{
    if (framebufferResizePending)
    {
        for (auto& callback : framebufferResizeCallbacks)
            callback.first(callback.second, this, currentFramebufferSize);

        framebufferResizePending = false;
    }

    if (windowResizePending)
    {
        for (auto& callback : windowResizeCallbacks)
            callback.first(callback.second, this, currentWindowSize);

        windowResizePending = false;
    }

    if (maximizeChangePending)
    {
        for (auto& callback : maximizeCallbacks)
            callback.first(callback.second, this, currentMaximizeState);

        maximizeChangePending = false;
    }
}

bool Window::GetShouldClose()
{
    return glfwWindowShouldClose(windowHandle) == GLFW_TRUE;
}

void Window::SetShouldClose(bool shouldClose)
{
    glfwSetWindowShouldClose(windowHandle, shouldClose ? GLFW_TRUE : GLFW_FALSE);
}

Vec2i Window::GetFrameBufferSize()
{
    return currentFramebufferSize;
}

Vec2i Window::GetWindowSize()
{
    int width, height;
    glfwGetWindowSize(windowHandle, &width, &height);

    return Vec2i(width, height);
}

float Window::GetScreenCoordinateScale()
{
    float x, y;
    glfwGetWindowContentScale(windowHandle, &x, &y);

    (void)y;

    return x;
}

Mat4x4f Window::GetScreenSpaceProjectionMatrix()
{
    Vec2i frame = this->GetFrameBufferSize();

    const float farClipDistance = 100.0f;
    const float nearClipDistance = -100.0f;

    const float farMinusNear = farClipDistance - nearClipDistance;
    const float farPlusNear = farClipDistance + nearClipDistance;

    Mat4x4f result;
    result[0] = 2.0f / frame.x;
    result[5] = -2.0f / frame.y;
    result[10] = -2.0f / (farMinusNear);
    result[12] = -1.0f;
    result[13] = 1.0f;
    result[14] = -(farPlusNear) / (farMinusNear);

    return result;
}
void Window::SetWindowTitle(const char* title)
{
    glfwSetWindowTitle(windowHandle, title);
}

void Window::SetCursorMode(CursorMode mode)
{
    int cursorModeValue = GLFW_CURSOR_NORMAL;

    if (mode == CursorMode::Hidden)
        cursorModeValue = GLFW_CURSOR_HIDDEN;
    else if (mode == CursorMode::Disabled)
        cursorModeValue = GLFW_CURSOR_DISABLED;

    glfwSetInputMode(windowHandle, GLFW_CURSOR, cursorModeValue);
}

Window::CursorMode Window::GetCursorMode() const
{
    int cursorModeValue = glfwGetInputMode(windowHandle, GLFW_CURSOR);

    CursorMode mode = CursorMode::Normal;

    if (cursorModeValue == GLFW_CURSOR_HIDDEN)
        mode = CursorMode::Hidden;
    else if (cursorModeValue == GLFW_CURSOR_DISABLED)
        mode = CursorMode::Disabled;

    return mode;
}

void Window::RegisterFramebufferResizeCallback(ResizeCallbackFn callback, void* userPointer)
{
    framebufferResizeCallbacks.PushBack(Pair(callback, userPointer));
}

void Window::UnregisterFramebufferResizeCallback(ResizeCallbackFn callback, void* userPointer)
{
    UnregisterCallback(framebufferResizeCallbacks, callback, userPointer);
}

void Window::RegisterWindowResizeCallback(ResizeCallbackFn callback, void* userPointer)
{
    windowResizeCallbacks.PushBack(Pair(callback, userPointer));
}

void Window::UnregisterWindowResizeCallback(ResizeCallbackFn callback, void* userPointer)
{
    UnregisterCallback(windowResizeCallbacks, callback, userPointer);
}

void Window::RegisterMaximizeCallback(ToggleCallbackFn callback, void* userPointer)
{
    maximizeCallbacks.PushBack(Pair(callback, userPointer));
}

void Window::UnregisterMaximizeCallback(ToggleCallbackFn callback, void* userPointer)
{
    UnregisterCallback(maximizeCallbacks, callback, userPointer);
}

Window* Window::GetWindowObject(GLFWwindow* windowHandle)
{
    return static_cast<Window*>(glfwGetWindowUserPointer(windowHandle));
}

template <typename CallbackType>
void Window::UnregisterCallback(Array<Pair<CallbackType, void*>>& arr, CallbackType callback, void* userPtr)
{
    int index = -1;

    for (size_t i = 0, count = arr.GetCount(); i < count; ++i)
    {
        const auto& cb = arr[i];
        if (cb.first == callback && cb.second == userPtr)
        {
            index = static_cast<int>(i);
            break;
        }
    }

    if (index >= 0)
    {
        // Swap the last item into the removed item's place
        if (static_cast<size_t>(index) < arr.GetCount() - 1)
            arr[index] = arr.GetBack();

        arr.PopBack();
    }
}

void Window::_GlfwFramebufferSizeCallback(GLFWwindow* window, int width, int height)
{
    GetWindowObject(window)->GlfwFramebufferSizeCallback(width, height);
}

void Window::_GlfwWindowSizeCallback(GLFWwindow* window, int width, int height)
{
    GetWindowObject(window)->GlfwWindowSizeCallback(width, height);
}

void Window::_GlfwMaximizeCallback(GLFWwindow* window, int maximized)
{
    GetWindowObject(window)->GlfwMaximizeCallback(maximized);
}

void Window::GlfwFramebufferSizeCallback(int width, int height)
{
    if ((width != 0 && height != 0) &&
        (width != currentFramebufferSize.x || height != currentFramebufferSize.y))
    {
        framebufferResizePending = true;

        currentFramebufferSize.x = width;
        currentFramebufferSize.y = height;
    }
}

void Window::GlfwWindowSizeCallback(int width, int height)
{
    if ((width != 0 && height != 0) &&
        (width != currentWindowSize.x || height != currentWindowSize.y))
    {
        windowResizePending = true;

        currentWindowSize.x = width;
        currentWindowSize.y = height;
    }
}

void Window::GlfwMaximizeCallback(int maximized)
{
    if ((maximized == GLFW_TRUE) != currentMaximizeState)
    {
        maximizeChangePending = true;

        currentMaximizeState = maximized == GLFW_TRUE;
    }
}
} // namespace kokko
