#include "System/ImGuiInputView.hpp"

#include "imgui.h"

ImGuiInputView::ImGuiInputView(InputView* source) :
	FilterInputView(source, "ImGuiInputView")
{
}

bool ImGuiInputView::WantsMouseInput()
{
	return ImGui::GetIO().WantCaptureMouse;
}

bool ImGuiInputView::WantsKeyboardInput()
{
	return ImGui::GetIO().WantCaptureKeyboard;
}

bool ImGuiInputView::WantsTextInput()
{
	return ImGui::GetIO().WantTextInput;
}
