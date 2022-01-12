#include "EditorApp.hpp"

#include <cmath>

#include "imgui.h"
#include "ImGuizmo.h"

#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

#include "Core/Core.hpp"

#include "Engine/Engine.hpp"
#include "Engine/EntityManager.hpp"
#include "Engine/World.hpp"

#include "Graphics/Scene.hpp"

#include "Math/Math.hpp"
#include "Math/Rectangle.hpp"

#include "Memory/Allocator.hpp"

#include "Rendering/CameraParameters.hpp"
#include "Rendering/CameraSystem.hpp"
#include "Rendering/LightManager.hpp"
#include "Rendering/RenderDevice.hpp"
#include "Rendering/Renderer.hpp"

#include "Resources/ResourceManagers.hpp"

#include "System/FilesystemVirtual.hpp"
#include "System/InputManager.hpp"
#include "System/Window.hpp"

#include "EditorConstants.hpp"
#include "EditorCore.hpp"
#include "EditorWindow.hpp"

namespace kokko
{
namespace editor
{

EditorApp::EditorApp(Allocator* allocator, FilesystemVirtual* filesystem) :
	engine(nullptr),
	virtualFilesystem(filesystem),
	allocator(allocator),
	renderDevice(nullptr),
	world(nullptr),
	core(nullptr),
	project(allocator),
	exitRequested(false),
	currentMainMenuDialog(MainMenuDialog::None),
	currentDialogId(0)
{
	core = allocator->MakeNew<EditorCore>(allocator, filesystem);
}

EditorApp::~EditorApp()
{
	allocator->MakeDelete(core);
}

void EditorApp::Initialize(Engine* engine)
{
	KOKKO_PROFILE_FUNCTION();

	this->engine = engine;
	this->renderDevice = engine->GetRenderDevice();
	this->world = engine->GetWorld();

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

	float scale = engine->GetMainWindow()->GetScreenCoordinateScale();

	// TODO: Scale spacing values
	// TODO: Update font size and spacing if window is moved to another screen

	{
		KOKKO_PROFILE_SCOPE("ImGui AddFontFromFileTTF()");

		float fontSize = std::floor(15.0f * scale);
		// TODO: Read from virtual filesystem
		io.Fonts->AddFontFromFileTTF("editor/res/fonts/roboto/Roboto-Regular.ttf", fontSize);
	}

	ImGui::StyleColorsDark();

	// Update style colors to linear space
	ImVec4* imguiColors = ImGui::GetStyle().Colors;
	for (size_t i = 0; i < ImGuiCol_COUNT; ++i)
	{
		ImVec4 color = imguiColors[i];
		color.x = Math::SrgbFloatToLinear(color.x);
		color.y = Math::SrgbFloatToLinear(color.y);
		color.z = Math::SrgbFloatToLinear(color.z);
		// Leave alpha as is

		imguiColors[i] = color;
	}

	GLFWwindow* glfwWindow = engine->GetMainWindow()->GetGlfwWindow();

	{
		KOKKO_PROFILE_SCOPE("ImGui_ImplGlfw_InitForOpenGL()");
		ImGui_ImplGlfw_InitForOpenGL(glfwWindow, true);
	}

	{
		KOKKO_PROFILE_SCOPE("ImGui_ImplOpenGL3_Init()");
		ImGui_ImplOpenGL3_Init();
	}

	core->Initialize(engine);

	if (userSettings.DeserializeFromFile(EditorConstants::UserSettingsFilePath) == false)
	{
		KK_LOG_INFO("Failed to open editor_user_settings.yml, should open project dialog.");
		return;
	}

	if (userSettings.lastOpenedProject.empty())
	{
		KK_LOG_INFO("Last opened project is empty, should open project dialog.");
		return;
	}

	if (OpenProject(userSettings.lastOpenedProject) == false)
	{
		std::string pathStr = userSettings.lastOpenedProject.u8string();
		KK_LOG_ERROR("Failed to open project.yml from path {}", pathStr.c_str());
		return;
	}

	if (userSettings.lastOpenedLevel.HasValue())
	{
		core->OpenLevel(userSettings.lastOpenedLevel.GetValue());
	}
}

void EditorApp::Deinitialize()
{
	KOKKO_PROFILE_FUNCTION();

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();

	ImGui::DestroyContext();
}

void EditorApp::StartFrame()
{
	KOKKO_PROFILE_FUNCTION();

	core->ResizeSceneViewFramebufferIfRequested();

	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();

	ImGui::NewFrame();
	ImGuizmo::BeginFrame();

	ImGuizmo::SetOrthographic(false);

	ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());
}

void EditorApp::Update(EngineSettings* engineSettings, bool& shouldExitOut)
{
	KOKKO_PROFILE_FUNCTION();

	DrawMainMenuBar();

	core->Update();

	//ImGui::ShowDemoWindow();

	if (exitRequested)
		shouldExitOut = true;
}

void EditorApp::EndFrame()
{
	KOKKO_PROFILE_FUNCTION();

	core->LateUpdate();

	core->EndFrame();

	{
		KOKKO_PROFILE_SCOPE("ImGui::Render()");
		ImGui::Render();
	}

	{
		KOKKO_PROFILE_SCOPE("Clear default framebuffer");

		renderDevice->BindFramebuffer(RenderFramebufferTarget::Framebuffer, 0);

		RenderCommandData::ClearColorData clearColor{ 0.0f, 0.0f, 0.0f, 1.0f };
		renderDevice->ClearColor(&clearColor);

		RenderCommandData::ClearMask clearMask{ true, false, false };
		renderDevice->Clear(&clearMask);
	}

	{
		KOKKO_PROFILE_SCOPE("ImGuiRenderBackend::RenderDrawData()");
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	}

	Optional<Uid> loadedLevel = core->GetLoadedLevelUid();
	if (loadedLevel.HasValue() &&
		(userSettings.lastOpenedLevel.HasValue() == false ||
		userSettings.lastOpenedLevel.GetValue() != loadedLevel.GetValue()))
	{
		userSettings.lastOpenedLevel = loadedLevel.GetValue();
		userSettings.SerializeToFile(EditorConstants::UserSettingsFilePath);
	}
}

const Framebuffer& EditorApp::GetSceneViewFramebuffer()
{
	return core->GetSceneViewFramebuffer();
}

AssetLibrary* EditorApp::GetAssetLibrary()
{
	return core->GetAssetLibrary();
}

CameraParameters EditorApp::GetEditorCameraParameters() const
{
	return core->GetEditorCameraParameters();
}

void EditorApp::DrawMainMenuBar()
{
	KOKKO_PROFILE_FUNCTION();

	bool createProject = false;
	bool openProject = false;
	bool saveLevelAs = false;

	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("New project..."))
				createProject = true;

			if (ImGui::MenuItem("Open project..."))
				openProject = true;

			ImGui::Separator();

			if (ImGui::MenuItem("New level"))
			{
				// TODO: Make sure level changes have been saved
				world->ClearAllEntities();
			}

			if (ImGui::MenuItem("Save level as..."))
				saveLevelAs = true;

			ImGui::Separator();

			if (ImGui::MenuItem("Exit"))
				exitRequested = true;

			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Edit"))
		{
			if (ImGui::MenuItem("Copy"))
			{
				core->CopyEntity();
			}

			if (ImGui::MenuItem("Paste"))
			{
				core->PasteEntity();
			}

			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("View"))
		{
			for (EditorWindow* window : core->GetWindows())
			{
				if (ImGui::MenuItem(window->windowTitle, nullptr))
				{
					window->windowIsOpen = true;
					window->requestFocus = true;
				}
			}

			ImGui::EndMenu();
		}

		ImGui::EndMainMenuBar();
	}

	if (createProject)
	{
		auto descriptor = FilePickerDialog::Descriptor{
			"Create project",
			"Select where to create the new project. The directory must be empty.",
			"Select directory",
			FilePickerDialog::Type::FolderOpen,
			false,
		};
		currentDialogId = filePicker.StartDialog(descriptor);
		currentMainMenuDialog = MainMenuDialog::CreateProject;
	}
	else if (openProject)
	{
		auto descriptor = FilePickerDialog::Descriptor{
			"Open project",
			"Select a project directory.",
			"Select directory",
			FilePickerDialog::Type::FolderOpen,
			false,
		};
		currentDialogId = filePicker.StartDialog(descriptor);
		currentMainMenuDialog = MainMenuDialog::OpenProject;
	}
	else if (saveLevelAs)
	{
		auto descriptor = FilePickerDialog::Descriptor{
			"Save level as",
			"Select where to save the level.",
			"Save",
			FilePickerDialog::Type::FileSave,
			true,
			project.GetAssetPath()
		};
		currentDialogId = filePicker.StartDialog(descriptor);
		currentMainMenuDialog = MainMenuDialog::SaveLevelAs;
	}

	filePicker.Update();

	std::filesystem::path filePickerPathOut;

	if (currentMainMenuDialog == MainMenuDialog::CreateProject &&
		filePicker.GetDialogResult(currentDialogId, filePickerPathOut))
	{
		ResetMainMenuDialog();

		if (filePickerPathOut.empty() == false)
		{
			std::string filenameStr = filePickerPathOut.filename().u8string();
			CreateProject(filePickerPathOut, StringRef(filenameStr.c_str(), filenameStr.length()));
		}
	}

	if (currentMainMenuDialog == MainMenuDialog::OpenProject &&
		filePicker.GetDialogResult(currentDialogId, filePickerPathOut))
	{
		ResetMainMenuDialog();

		if (filePickerPathOut.empty() == false)
		{
			OpenProject(filePickerPathOut);
		}
	}

	if (currentMainMenuDialog == MainMenuDialog::SaveLevelAs &&
		filePicker.GetDialogResult(currentDialogId, filePickerPathOut))
	{
		ResetMainMenuDialog();

		if (filePickerPathOut.empty() == false)
		{
			core->SaveLevelAs(filePickerPathOut);
		}
	}
}

void EditorApp::ResetMainMenuDialog()
{
	currentMainMenuDialog = MainMenuDialog::None;
	currentDialogId = 0;
}

bool EditorApp::CreateProject(const std::filesystem::path& directory, StringRef name)
{
	namespace fs = std::filesystem;

	std::error_code err;
	fs::file_status stat = fs::status(directory, err);

	if (err)
		return false;

	if (fs::is_directory(stat) == false)
		return false;

	// Check that the directory is empty
	for (const auto& entry : fs::directory_iterator(directory))
		return false;

	// Create assets folder
	if (fs::create_directory(directory / EditorConstants::AssetDirectoryName) == false)
		return false;

	project.SetRootPath(directory);
	project.SetName(name);

	if (project.SerializeToFile() == false)
	{
		project.SetRootPath(fs::path());
		project.SetName(StringRef());
		return false;
	}

	OnProjectChanged();

	return true;
}

bool EditorApp::OpenProject(const std::filesystem::path& projectDir)
{
	if (project.DeserializeFromFile(projectDir))
	{
		OnProjectChanged();

		return true;
	}

	return false;
}

void EditorApp::OnProjectChanged()
{
	userSettings.lastOpenedProject = project.GetRootPath();
	if (userSettings.SerializeToFile(EditorConstants::UserSettingsFilePath) == false)
		KK_LOG_ERROR("Failed to save settings to {}", EditorConstants::UserSettingsFilePath);

	std::filesystem::path assetPath = project.GetRootPath() / "assets";
	std::string assetPathStr = assetPath.u8string();
	StringRef assetPathRef(assetPathStr.c_str(), assetPathStr.length());

	FilesystemVirtual::MountPoint mounts[] = {
		FilesystemVirtual::MountPoint{ StringRef(EditorConstants::VirtualMountEngine), StringRef("engine/res") },
		FilesystemVirtual::MountPoint{ StringRef(EditorConstants::VirtualMountEditor), StringRef("editor/res") },
		FilesystemVirtual::MountPoint{ StringRef(EditorConstants::VirtualMountAssets), assetPathRef }
	};
	virtualFilesystem->SetMountPoints(ArrayView(mounts));

	const String& name = project.GetName();

	KK_LOG_INFO("Editor project changed to {}", name.GetCStr());

	engine->GetMainWindow()->SetWindowTitle(name.GetCStr());

	core->NotifyProjectChanged(&project);
}

}
}
