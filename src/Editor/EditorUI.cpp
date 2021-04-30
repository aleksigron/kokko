#include "Editor/EditorUI.hpp"

#include "imgui.h"

#include "Core/Core.hpp"

#include "Editor/EditorViews.hpp"

#include "Engine/Engine.hpp"

#include "Math/Rectangle.hpp"

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

	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("New"))
			{
			}

			if (ImGui::MenuItem("Open", "Ctrl+O"))
			{
			}

			if (ImGui::MenuItem("Save", "Ctrl+S"))
			{
			}

			if (ImGui::MenuItem("Save As.."))
			{
			}

			ImGui::EndMenu();
		}
		ImGui::EndMainMenuBar();
	}

	views->entityView.Draw();

	ImGui::ShowDemoWindow();

	ImGui::Render();
	renderBackend->RenderDrawData(ImGui::GetDrawData());
}

ViewRectangle EditorUI::GetWorldViewport()
{
	ImGuiViewport* viewport = ImGui::GetMainViewport();

	Vec2i pos = Vec2f(viewport->WorkPos.x, viewport->WorkPos.y).As<int>();
	Vec2i size = Vec2f(viewport->WorkSize.x, viewport->WorkSize.y).As<int>();
	Vec2i fullSize = Vec2f(viewport->Size.x, viewport->Size.y).As<int>();

	ViewRectangle rect;
	rect.position = fullSize - size - pos;
	rect.size = size;

	return rect;
}
