#include "ImGuiPlatformBackend.hpp"

#if defined(_MSC_VER) && !defined(_CRT_SECURE_NO_WARNINGS)
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <cstdio>
#include <cstddef>

#include "System/IncludeOpenGL.hpp"
#include "System/InputView.hpp"
#include "System/Time.hpp"
#include "System/KeyCode.hpp"

#include "imgui.h"

static const char* ImGui_ImplGlfw_GetClipboardText(void* user_data)
{
	return glfwGetClipboardString((GLFWwindow*)user_data);
}

static void ImGui_ImplGlfw_SetClipboardText(void* user_data, const char* text)
{
	glfwSetClipboardString((GLFWwindow*)user_data, text);
}

ImGuiPlatformBackend::ImGuiPlatformBackend() :
	glfwWindow(nullptr),
	inputView(nullptr)
{
}

ImGuiPlatformBackend::~ImGuiPlatformBackend()
{

}

bool ImGuiPlatformBackend::Initialize(GLFWwindow* window, InputView* inputView)
{
	this->glfwWindow = window;
	this->inputView = inputView;

	// Setup backend capabilities flags
	ImGuiIO& io = ImGui::GetIO();

	io.BackendPlatformName = "ImGuiPlatformBackend";
	// We can honor GetMouseCursor() values (optional)
	io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;

	// We can honor io.WantSetMousePos requests (optional, rarely used)
	io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;

	float xScale, yScale;
	glfwGetWindowContentScale(window, &xScale, &yScale);

	// TODO: Scale spacing values
	// TODO: Update font size and spacing if window is moved to another screen

	float fontSize(int(15.0f * xScale));
	io.Fonts->AddFontFromFileTTF("res/fonts/roboto/Roboto-Regular.ttf", fontSize);

	// Keyboard mapping. Dear ImGui will use those indices to peek into the io.KeysDown[] array.
	io.KeyMap[ImGuiKey_Tab] = static_cast<int>(KeyCode::Tab);
	io.KeyMap[ImGuiKey_LeftArrow] = static_cast<int>(KeyCode::Left);
	io.KeyMap[ImGuiKey_RightArrow] = static_cast<int>(KeyCode::Right);
	io.KeyMap[ImGuiKey_UpArrow] = static_cast<int>(KeyCode::Up);
	io.KeyMap[ImGuiKey_DownArrow] = static_cast<int>(KeyCode::Down);
	io.KeyMap[ImGuiKey_PageUp] = static_cast<int>(KeyCode::PageUp);
	io.KeyMap[ImGuiKey_PageDown] = static_cast<int>(KeyCode::PageDown);
	io.KeyMap[ImGuiKey_Home] = static_cast<int>(KeyCode::Home);
	io.KeyMap[ImGuiKey_End] = static_cast<int>(KeyCode::End);
	io.KeyMap[ImGuiKey_Insert] = static_cast<int>(KeyCode::Insert);
	io.KeyMap[ImGuiKey_Delete] = static_cast<int>(KeyCode::Delete);
	io.KeyMap[ImGuiKey_Backspace] = static_cast<int>(KeyCode::Backspace);
	io.KeyMap[ImGuiKey_Space] = static_cast<int>(KeyCode::Space);
	io.KeyMap[ImGuiKey_Enter] = static_cast<int>(KeyCode::Enter);
	io.KeyMap[ImGuiKey_Escape] = static_cast<int>(KeyCode::Escape);
	io.KeyMap[ImGuiKey_KeyPadEnter] = static_cast<int>(KeyCode::Enter);
	io.KeyMap[ImGuiKey_A] = static_cast<int>(KeyCode::A);
	io.KeyMap[ImGuiKey_C] = static_cast<int>(KeyCode::C);
	io.KeyMap[ImGuiKey_V] = static_cast<int>(KeyCode::V);
	io.KeyMap[ImGuiKey_X] = static_cast<int>(KeyCode::X);
	io.KeyMap[ImGuiKey_Y] = static_cast<int>(KeyCode::Y);
	io.KeyMap[ImGuiKey_Z] = static_cast<int>(KeyCode::Z);

	io.SetClipboardTextFn = ImGui_ImplGlfw_SetClipboardText;
	io.GetClipboardTextFn = ImGui_ImplGlfw_GetClipboardText;
	io.ClipboardUserData = glfwWindow;

	// Create mouse cursors
	// (By design, on X11 cursors are user configurable and some cursors may be missing. When a cursor doesn't exist,
	// GLFW will emit an error which will often be printed by the app, so we temporarily disable error reporting.
	// Missing cursors will return NULL and our _UpdateMouseCursor() function will use the Arrow cursor instead.)
	GLFWerrorfun prev_error_callback = glfwSetErrorCallback(NULL);
	mouseCursors[ImGuiMouseCursor_Arrow] = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
	mouseCursors[ImGuiMouseCursor_TextInput] = glfwCreateStandardCursor(GLFW_IBEAM_CURSOR);
	mouseCursors[ImGuiMouseCursor_ResizeNS] = glfwCreateStandardCursor(GLFW_VRESIZE_CURSOR);
	mouseCursors[ImGuiMouseCursor_ResizeEW] = glfwCreateStandardCursor(GLFW_HRESIZE_CURSOR);
	mouseCursors[ImGuiMouseCursor_Hand] = glfwCreateStandardCursor(GLFW_HAND_CURSOR);

	mouseCursors[ImGuiMouseCursor_ResizeAll] = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
	mouseCursors[ImGuiMouseCursor_ResizeNESW] = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
	mouseCursors[ImGuiMouseCursor_ResizeNWSE] = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
	mouseCursors[ImGuiMouseCursor_NotAllowed] = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);

	glfwSetErrorCallback(prev_error_callback);

	return true;
}

void ImGuiPlatformBackend::Deinitialize()
{
	for (ImGuiMouseCursor cursor_n = 0; cursor_n < CursorCount; cursor_n++)
	{
		glfwDestroyCursor(mouseCursors[cursor_n]);
		mouseCursors[cursor_n] = NULL;
	}
}

void ImGuiPlatformBackend::NewFrame()
{
	ImGuiIO& io = ImGui::GetIO();
	IM_ASSERT(io.Fonts->IsBuilt() && "Font atlas not built! It is generally built by the renderer backend. Missing call to renderer _NewFrame() function? e.g. ImGui_ImplOpenGL3_NewFrame().");

	// Setup display size (every frame to accommodate for window resizing)
	int w, h;
	int display_w, display_h;
	glfwGetWindowSize(glfwWindow, &w, &h);
	glfwGetFramebufferSize(glfwWindow, &display_w, &display_h);
	io.DisplaySize = ImVec2((float)w, (float)h);
	if (w > 0 && h > 0)
		io.DisplayFramebufferScale = ImVec2((float)display_w / w, (float)display_h / h);

	// Setup time step
	
	io.DeltaTime = Time::GetDeltaTime();

	ImGuiPlatformBackend::UpdateMousePosAndButtons();
	ImGuiPlatformBackend::UpdateMouseCursor();

	// Update game controllers (if enabled and available)
	ImGuiPlatformBackend::UpdateGamepads();

	for (int i = 0, count = inputView->GetInputCharCount(); i < count; ++i)
		io.AddInputCharacter(inputView->GetInputChar(i));

	// Clear key state
	for (size_t i = 0, count = sizeof(io.KeysDown); i < count; ++i)
		io.KeysDown[i] = false;

	// Set keys down
	for (int i = 0, count = inputView->GetActiveKeyCount(); i < count; ++i)
	{
		KeyCode key = inputView->GetActiveKeyCode(i);
		if (inputView->GetKey(key) == true)
			io.KeysDown[static_cast<size_t>(key)] = true;
	}

	// Set modifier keys
	io.KeyCtrl = io.KeysDown[static_cast<int>(KeyCode::LeftControl)] || io.KeysDown[static_cast<int>(KeyCode::RightControl)];
	io.KeyShift = io.KeysDown[static_cast<int>(KeyCode::LeftShift)] || io.KeysDown[static_cast<int>(KeyCode::RightShift)];
	io.KeyAlt = io.KeysDown[static_cast<int>(KeyCode::LeftAlt)] || io.KeysDown[static_cast<int>(KeyCode::RightAlt)];
#ifdef _WIN32
	io.KeySuper = false;
#else
	io.KeySuper = io.KeysDown[static_cast<int>(KeyCode::LeftSuper)] || io.KeysDown[static_cast<int>(KeyCode::RightSuper)];
#endif
}

void ImGuiPlatformBackend::UpdateMousePosAndButtons()
{
	// Update buttons
	ImGuiIO& io = ImGui::GetIO();
	for (int i = 0; i < IM_ARRAYSIZE(io.MouseDown); i++)
	{
		io.MouseDown[i] = inputView->GetMouseButton(i);
	}

	Vec2f scroll = inputView->GetScrollMovement();
	io.MouseWheelH = scroll.x;
	io.MouseWheel = scroll.y;

	// Update mouse position
	const ImVec2 mouse_pos_backup = io.MousePos;
	io.MousePos = ImVec2(-FLT_MAX, -FLT_MAX);
	const bool focused = glfwGetWindowAttrib(glfwWindow, GLFW_FOCUSED) != 0;
	if (focused)
	{
		if (io.WantSetMousePos)
		{
			glfwSetCursorPos(glfwWindow, (double)mouse_pos_backup.x, (double)mouse_pos_backup.y);
		}
		else
		{
			Vec2f mousePos = inputView->GetCursorPosition();
			io.MousePos = ImVec2(mousePos.x, mousePos.y);
		}
	}
}

void ImGuiPlatformBackend::UpdateMouseCursor()
{
	ImGuiIO& io = ImGui::GetIO();
	if ((io.ConfigFlags & ImGuiConfigFlags_NoMouseCursorChange) || glfwGetInputMode(glfwWindow, GLFW_CURSOR) == GLFW_CURSOR_DISABLED)
		return;

	ImGuiMouseCursor imgui_cursor = ImGui::GetMouseCursor();
	if (imgui_cursor == ImGuiMouseCursor_None || io.MouseDrawCursor)
	{
		// Hide OS mouse cursor if imgui is drawing it or if it wants no cursor
		glfwSetInputMode(glfwWindow, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
	}
	else
	{
		// Show OS mouse cursor
		// FIXME-PLATFORM: Unfocused windows seems to fail changing the mouse cursor with GLFW 3.2, but 3.3 works here.
		glfwSetCursor(glfwWindow, mouseCursors[imgui_cursor] ? mouseCursors[imgui_cursor] : mouseCursors[ImGuiMouseCursor_Arrow]);
		glfwSetInputMode(glfwWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	}
}

void ImGuiPlatformBackend::UpdateGamepads()
{
	ImGuiIO& io = ImGui::GetIO();
	memset(io.NavInputs, 0, sizeof(io.NavInputs));
	if ((io.ConfigFlags & ImGuiConfigFlags_NavEnableGamepad) == 0)
		return;

	// Update gamepad inputs
#define MAP_BUTTON(NAV_NO, BUTTON_NO)       { if (buttons_count > BUTTON_NO && buttons[BUTTON_NO] == GLFW_PRESS) io.NavInputs[NAV_NO] = 1.0f; }
#define MAP_ANALOG(NAV_NO, AXIS_NO, V0, V1) { float v = (axes_count > AXIS_NO) ? axes[AXIS_NO] : V0; v = (v - V0) / (V1 - V0); if (v > 1.0f) v = 1.0f; if (io.NavInputs[NAV_NO] < v) io.NavInputs[NAV_NO] = v; }
	int axes_count = 0, buttons_count = 0;
	const float* axes = glfwGetJoystickAxes(GLFW_JOYSTICK_1, &axes_count);
	const unsigned char* buttons = glfwGetJoystickButtons(GLFW_JOYSTICK_1, &buttons_count);
	MAP_BUTTON(ImGuiNavInput_Activate, 0);     // Cross / A
	MAP_BUTTON(ImGuiNavInput_Cancel, 1);     // Circle / B
	MAP_BUTTON(ImGuiNavInput_Menu, 2);     // Square / X
	MAP_BUTTON(ImGuiNavInput_Input, 3);     // Triangle / Y
	MAP_BUTTON(ImGuiNavInput_DpadLeft, 13);    // D-Pad Left
	MAP_BUTTON(ImGuiNavInput_DpadRight, 11);    // D-Pad Right
	MAP_BUTTON(ImGuiNavInput_DpadUp, 10);    // D-Pad Up
	MAP_BUTTON(ImGuiNavInput_DpadDown, 12);    // D-Pad Down
	MAP_BUTTON(ImGuiNavInput_FocusPrev, 4);     // L1 / LB
	MAP_BUTTON(ImGuiNavInput_FocusNext, 5);     // R1 / RB
	MAP_BUTTON(ImGuiNavInput_TweakSlow, 4);     // L1 / LB
	MAP_BUTTON(ImGuiNavInput_TweakFast, 5);     // R1 / RB
	MAP_ANALOG(ImGuiNavInput_LStickLeft, 0, -0.3f, -0.9f);
	MAP_ANALOG(ImGuiNavInput_LStickRight, 0, +0.3f, +0.9f);
	MAP_ANALOG(ImGuiNavInput_LStickUp, 1, +0.3f, +0.9f);
	MAP_ANALOG(ImGuiNavInput_LStickDown, 1, -0.3f, -0.9f);
#undef MAP_BUTTON
#undef MAP_ANALOG
	if (axes_count > 0 && buttons_count > 0)
		io.BackendFlags |= ImGuiBackendFlags_HasGamepad;
	else
		io.BackendFlags &= ~ImGuiBackendFlags_HasGamepad;
}
