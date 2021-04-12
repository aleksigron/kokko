#include "Editor/EditorUI.hpp"

#include "imgui.h"

#include "Memory/Allocator.hpp"

#include "System/ImGuiRenderBackend.hpp"
#include "System/ImGuiPlatformBackend.hpp"

EditorUI::EditorUI(Allocator* allocator) :
	allocator(allocator),
	renderBackend(nullptr),
	platformBackend(nullptr)
{

}

EditorUI::~EditorUI()
{

}

void EditorUI::Initialize(GLFWwindow* window, InputView* imguiInputView)
{
	renderBackend = allocator->MakeNew<ImGuiRenderBackend>();
	platformBackend = allocator->MakeNew<ImGuiPlatformBackend>();

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;

	ImGui::StyleColorsDark();

	renderBackend->Initialize();
	platformBackend->Initialize(window, imguiInputView);
}

void EditorUI::Deinitialize()
{
	platformBackend->Deinitialize();
	renderBackend->Deinitialize();

	ImGui::DestroyContext();

	allocator->MakeDelete(platformBackend);
	allocator->MakeDelete(renderBackend);
}

void EditorUI::StartFrame()
{
	renderBackend->NewFrame();
	platformBackend->NewFrame();

	ImGui::NewFrame();
}

void EditorUI::Render()
{
	ImGui::Render();
	renderBackend->RenderDrawData(ImGui::GetDrawData());
}
