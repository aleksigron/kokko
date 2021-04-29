#include "Editor/EditorUI.hpp"

#include "imgui.h"

#include "Core/Core.hpp"

#include "Editor/EditorViews.hpp"

#include "Engine/Engine.hpp"

#include "Memory/Allocator.hpp"

#include "System/ImGuiRenderBackend.hpp"
#include "System/ImGuiPlatformBackend.hpp"
#include "System/InputManager.hpp"
#include "System/Window.hpp"

EditorUI::EditorUI(Allocator* allocator) :
	allocator(allocator),
	renderBackend(nullptr),
	platformBackend(nullptr)
{
	views = allocator->MakeNew<EditorViews>();
}

EditorUI::~EditorUI()
{
	allocator->MakeDelete(views);
}

void EditorUI::Initialize(Engine* engine)
{
	KOKKO_PROFILE_FUNCTION();

	renderBackend = allocator->MakeNew<ImGuiRenderBackend>();
	platformBackend = allocator->MakeNew<ImGuiPlatformBackend>();

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;

	ImGui::StyleColorsDark();

	renderBackend->Initialize();

	Window* window = engine->GetMainWindow();
	platformBackend->Initialize(window->GetGlfwWindow(), window->GetInputManager()->GetImGuiInputView());

	views->entityView.Initialize(engine);
}

void EditorUI::Deinitialize()
{
	KOKKO_PROFILE_FUNCTION();

	platformBackend->Deinitialize();
	renderBackend->Deinitialize();

	ImGui::DestroyContext();

	allocator->MakeDelete(platformBackend);
	allocator->MakeDelete(renderBackend);
}

void EditorUI::StartFrame()
{
	KOKKO_PROFILE_FUNCTION();

	renderBackend->NewFrame();
	platformBackend->NewFrame();

	ImGui::NewFrame();
}

void EditorUI::Render()
{
	KOKKO_PROFILE_FUNCTION();

	views->entityView.Draw();

	ImGui::ShowDemoWindow();

	ImGui::Render();
	renderBackend->RenderDrawData(ImGui::GetDrawData());
}
