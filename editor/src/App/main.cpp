#include "doctest/doctest.h"

#include "Debug/Instrumentation.hpp"

#include "Engine/Engine.hpp"
#include "Engine/EngineConstants.hpp"

#include "Memory/RootAllocator.hpp"
#include "Memory/AllocatorManager.hpp"

#include "Platform/Window.hpp"

#include "Rendering/CameraParameters.hpp"

#include "Resources/AssetLibrary.hpp"

#include "System/Filesystem.hpp"
#include "System/FilesystemResolverVirtual.hpp"
#include "System/Logger.hpp"
#include "System/WindowManager.hpp"
#include "System/WindowSettings.hpp"

#include "ConsoleLogger.hpp"
#include "EditorApp.hpp"
#include "EditorAssetLoader.hpp"
#include "EditorConstants.hpp"

namespace
{
kokko::WindowSettings GetWindowSettings(const kokko::editor::EditorUserSettings& userSettings)
{
	kokko::WindowSettings windowSettings;
	windowSettings.width = userSettings.windowWidth;
	windowSettings.height = userSettings.windowHeight;
	windowSettings.maximized = userSettings.windowMaximized;
	windowSettings.title = "Kokko Editor";

	if (windowSettings.width == 0)
		windowSettings.width = 1920;
	if (windowSettings.width < 600)
		windowSettings.width = 600;
	if (windowSettings.width > (1 << 14))
		windowSettings.width = (1 << 14);

	if (windowSettings.height == 0)
		windowSettings.height = 1080;
	if (windowSettings.height < 400)
		windowSettings.height = 400;
	if (windowSettings.height > (1 << 14))
		windowSettings.height = (1 << 14);

	return windowSettings;
}

}

int main(int argc, char** argv)
{
	RootAllocator rootAllocator;
	Allocator* defaultAlloc = RootAllocator::GetDefaultAllocator();
	kokko::Logger logger(defaultAlloc);
	kokko::Log::SetLogInstance(&logger);
	kokko::editor::ConsoleLogger consoleLogger(defaultAlloc);
	logger.SetReceiver(&consoleLogger);

	Instrumentation& instr = Instrumentation::Get();
	instr.BeginSession("unit_test_trace.json");

	doctest::Context ctx;
	ctx.setOption("abort-after", 5);
	ctx.applyCommandLine(argc, argv);
	ctx.setOption("no-breaks", true);
	int res = ctx.run();
	if (ctx.shouldExit())
		return res;

	instr.EndSession();
	instr.BeginSession("startup_trace.json");

	AllocatorManager* allocManager = defaultAlloc->MakeNew<AllocatorManager>(defaultAlloc);

	// Virtual filesystem
	// Initial virtual mount points before editor project is loaded

	Allocator* filesystemAllocator = allocManager->CreateAllocatorScope("Filesystem", defaultAlloc);
	kokko::FilesystemResolverVirtual resolver(filesystemAllocator);
	kokko::Filesystem filesystem(filesystemAllocator, &resolver);

	using EngineConst = kokko::EngineConstants;
	using EditorConst = kokko::editor::EditorConstants;

	kokko::FilesystemResolverVirtual::MountPoint mounts[] = {
		kokko::FilesystemResolverVirtual::MountPoint{
			kokko::ConstStringView(EngineConst::VirtualMountEngine),
			kokko::ConstStringView(EngineConst::EngineResourcePath)
		},
		kokko::FilesystemResolverVirtual::MountPoint{
			kokko::ConstStringView(EditorConst::VirtualMountEditor),
			kokko::ConstStringView(EditorConst::EditorResourcePath)
		}
	};
	resolver.SetMountPoints(kokko::ArrayView(mounts));

	Allocator* appAllocator = allocManager->CreateAllocatorScope("EditorApp", defaultAlloc);
	kokko::editor::EditorApp editor(appAllocator, &filesystem, &resolver);
	kokko::AssetLibrary* assetLibrary = editor.GetAssetLibrary();

	editor.LoadUserSettings();
	kokko::WindowSettings windowSettings = GetWindowSettings(editor.GetUserSettings());

	// TODO: Make sure EditorApp GPU resources are released before Engine is destroyed

	kokko::editor::EditorAssetLoader assetLoader(appAllocator, &filesystem, assetLibrary);

	// Engine

	Engine engine(allocManager, &filesystem, &assetLoader);

	if (assetLibrary->ScanAssets(true, true, false) == false)
	{
		instr.EndSession();

		res = -1;
	}
	else if (engine.Initialize(windowSettings))
	{
		kokko::render::CommandEncoder* commandEncoder = engine.GetCommandEncoder();

		editor.Initialize(&engine, &consoleLogger);

		engine.SetAppPointer(&editor);

		instr.EndSession();

		while (engine.GetWindowManager()->GetWindow()->GetShouldClose() == false)
		{
			engine.StartFrame();
			editor.StartFrame();

			engine.UpdateWorld();

			// Because editor can change the state of the world and systems,
			// let's run those updates at the same part of the frame as other updates
			bool editorWantsToExit = false;
			editor.Update(engine.GetSettings(), editorWantsToExit);
			if (editorWantsToExit)
                engine.GetWindowManager()->GetWindow()->SetShouldClose(true);

			engine.Render(editor.GetEditorCameraParameters(), editor.GetSceneViewFramebuffer());

			editor.EndFrame(commandEncoder);
			engine.EndFrame();
		}

		editor.Deinitialize();
	}
	else
	{
		instr.EndSession();

		res = -1;
	}

	return res;
}
	
