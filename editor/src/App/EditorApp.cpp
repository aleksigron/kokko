#include "EditorApp.hpp"

#include <cmath>

#include "imgui.h"
#include "ImGuizmo.h"

#include "backends/imgui_impl_glfw.h"

#include "Core/Core.hpp"

#include "Engine/Engine.hpp"
#include "Engine/EngineConstants.hpp"
#include "Engine/EntityManager.hpp"
#include "Engine/World.hpp"

#include "Graphics/Scene.hpp"

#include "Math/Math.hpp"
#include "Math/Rectangle.hpp"

#include "Memory/Allocator.hpp"

#include "Platform/Window.hpp"

#include "Rendering/CommandEncoder.hpp"
#include "Rendering/CameraParameters.hpp"
#include "Rendering/CameraSystem.hpp"
#include "Rendering/LightManager.hpp"
#include "Rendering/RenderDevice.hpp"
#include "Rendering/Renderer.hpp"

#include "Resources/ResourceManagers.hpp"

#include "System/Filesystem.hpp"
#include "System/FilesystemResolverVirtual.hpp"
#include "System/InputManager.hpp"
#include "System/WindowManager.hpp"

#include "EditorConstants.hpp"
#include "EditorCore.hpp"
#include "EditorWindow.hpp"
#include "ImguiImplOpenGL.hpp"

namespace kokko
{
namespace editor
{

EditorApp::EditorApp(Allocator* allocator, Filesystem* filesystem, FilesystemResolverVirtual* resolver) :
	engine(nullptr),
	filesystemResolver(resolver),
	allocator(allocator),
	renderDevice(nullptr),
	world(nullptr),
	project(allocator),
	exitRequested(false),
	currentMainMenuDialog(MainMenuDialog::None),
	currentDialogId(0)
{
	imguiImplOpenGL = kokko::MakeUnique<ImguiImplOpenGL>(allocator, allocator);
	core = kokko::MakeUnique<EditorCore>(allocator, allocator, filesystem, resolver);
}

EditorApp::~EditorApp()
{
}

void EditorApp::LoadUserSettings()
{
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
}

const EditorUserSettings& EditorApp::GetUserSettings() const
{
	return userSettings;
}

void EditorApp::Initialize(Engine* engine, ConsoleLogger* consoleLogger)
{
	KOKKO_PROFILE_FUNCTION();

#ifdef KOKKO_USE_METAL
    return;
#endif

	this->engine = engine;
	this->renderDevice = engine->GetRenderDevice();
	this->world = engine->GetWorld();

	Window* window = engine->GetWindowManager()->GetWindow();
	window->RegisterWindowResizeCallback(OnWindowResize, this);
	window->RegisterMaximizeCallback(OnWindowMaximize, this);

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

	float scale = window->GetScreenCoordinateScale();

	// TODO: Scale spacing values
	// TODO: Update font size and spacing if window is moved to another screen

	{
		KOKKO_PROFILE_SCOPE("ImGui AddFontFromFileTTF()");

		float regularFontSize = std::floor(15.0f * scale);
		float monoFontSize = std::floor(16.0f * scale);
		// TODO: Read from virtual filesystem
		io.Fonts->AddFontFromFileTTF("editor/res/fonts/roboto/Roboto-Medium.ttf", regularFontSize);
		io.Fonts->AddFontFromFileTTF("editor/res/fonts/roboto-mono/RobotoMono-Medium.ttf", monoFontSize);
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

	GLFWwindow* glfwWindow = engine->GetWindowManager()->GetWindow()->GetGlfwWindow();

	{
		KOKKO_PROFILE_SCOPE("ImGui_ImplGlfw_InitForOpenGL()");
		ImGui_ImplGlfw_InitForOpenGL(glfwWindow, true);
	}

	{
		KOKKO_PROFILE_SCOPE("ImguiImplOpenGL::Initialize()");
		imguiImplOpenGL->Initialize();
	}

	core->Initialize(engine, consoleLogger);

	if (userSettings.lastOpenedProject.empty())
		return;

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

#ifdef KOKKO_USE_METAL
    return;
#endif
	imguiImplOpenGL->Deinitialize();
	ImGui_ImplGlfw_Shutdown();

	ImGui::DestroyContext();

	core->Deinitialize();
}

void EditorApp::StartFrame()
{
	KOKKO_PROFILE_FUNCTION();

#ifdef KOKKO_USE_METAL
    return;
#endif
	core->ResizeSceneViewFramebufferIfRequested();

	imguiImplOpenGL->NewFrame();
	ImGui_ImplGlfw_NewFrame();

	ImGui::NewFrame();
	ImGuizmo::BeginFrame();

	ImGuizmo::SetOrthographic(false);

	ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());
}

void EditorApp::Update(kokko::EngineSettings* engineSettings, bool& shouldExitOut)
{
	KOKKO_PROFILE_FUNCTION();

#ifdef KOKKO_USE_METAL
    return;
#endif
	DrawMainMenuBar();

	core->Update();

	//ImGui::ShowDemoWindow();

	if (exitRequested)
		shouldExitOut = true;
}

void EditorApp::EndFrame(render::CommandEncoder* encoder)
{
	KOKKO_PROFILE_FUNCTION();

#ifdef KOKKO_USE_METAL
    return;
#endif
	core->LateUpdate();

	core->EndFrame();

	{
		KOKKO_PROFILE_SCOPE("ImGui::Mesh()");
		ImGui::Render();
	}

	{
		KOKKO_PROFILE_SCOPE("Clear default framebuffer");

		encoder->BindFramebuffer(render::FramebufferId());
		encoder->SetClearColor(Vec4f{ 0.0f, 0.0f, 0.0f, 1.0f });
		encoder->Clear(ClearMask{ true, false, false });
	}

	{
		KOKKO_PROFILE_SCOPE("ImGuiRenderBackend::RenderDrawData()");
		imguiImplOpenGL->RenderDrawData(encoder, ImGui::GetDrawData());
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

const render::Framebuffer& EditorApp::GetSceneViewFramebuffer()
{
	return core->GetSceneViewFramebuffer();
}

AssetLibrary* EditorApp::GetAssetLibrary()
{
	return core->GetAssetLibrary();
}

CameraParameters EditorApp::GetEditorCameraParameters() const
{
#ifdef KOKKO_USE_METAL
    return CameraParameters{};
#endif
	return core->GetEditorCameraParameters();
}

void EditorApp::DrawMainMenuBar()
{
	KOKKO_PROFILE_FUNCTION();

	bool createProject = false;
	bool openProject = false;
	bool saveLevel = false;
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
				core->NewLevel();
			}

			if (ImGui::MenuItem("Save level"))
				saveLevel = true;

			if (ImGui::MenuItem("Save level as..."))
				saveLevelAs = true;

			ImGui::Separator();

			if (ImGui::MenuItem("Exit"))
				exitRequested = true;

			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Edit"))
		{
			if (ImGui::MenuItem("Copy entity"))
			{
				core->CopyEntity();
			}

			if (ImGui::MenuItem("Paste entity"))
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

	if (saveLevel && core->GetLoadedLevelUid().HasValue() == false)
	{
		saveLevel = false;
		saveLevelAs = true;
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
	else if (saveLevel)
	{
		core->SaveLevel();
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
			CreateProject(filePickerPathOut, ConstStringView(filenameStr.c_str(), filenameStr.length()));
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

bool EditorApp::CreateProject(const std::filesystem::path& directory, ConstStringView name)
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
		project.SetName(ConstStringView());
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
	ConstStringView assetPathRef(assetPathStr.c_str(), assetPathStr.length());

	using MountPoint = FilesystemResolverVirtual::MountPoint;
	MountPoint mounts[] = {
		MountPoint{
			ConstStringView(EngineConstants::VirtualMountEngine),
			ConstStringView(EngineConstants::EngineResourcePath)
		},
		MountPoint{
			ConstStringView(EditorConstants::VirtualMountEditor),
			ConstStringView(EngineConstants::EngineResourcePath)
		},
		MountPoint{
			ConstStringView(EngineConstants::VirtualMountAssets),
			assetPathRef
		}
	};
	filesystemResolver->SetMountPoints(ArrayView(mounts));

	const String& name = project.GetName();

	KK_LOG_INFO("Editor project changed to {}", name.GetCStr());

	String windowTitle = name + " - Kokko Engine";
	engine->GetWindowManager()->GetWindow()->SetWindowTitle(windowTitle.GetCStr());

	core->NotifyProjectChanged(&project);
}

void EditorApp::OnWindowResize(void* app, Window* window, Vec2i size)
{
	EditorApp* editorApp = static_cast<EditorApp*>(app);
	editorApp->userSettings.windowWidth = size.x;
	editorApp->userSettings.windowHeight = size.y;
	editorApp->userSettings.SerializeToFile(EditorConstants::UserSettingsFilePath);
}

void EditorApp::OnWindowMaximize(void* app, Window* window, bool maximized)
{
	EditorApp* editorApp = static_cast<EditorApp*>(app);
	editorApp->userSettings.windowMaximized = maximized;
	editorApp->userSettings.SerializeToFile(EditorConstants::UserSettingsFilePath);
}

}
}
