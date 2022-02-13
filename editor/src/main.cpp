#include "doctest/doctest.h"

#include "Debug/Instrumentation.hpp"

#include "Engine/Engine.hpp"

#include "Memory/Memory.hpp"
#include "Memory/AllocatorManager.hpp"

#include "Rendering/CameraParameters.hpp"

#include "System/FilesystemVirtual.hpp"
#include "System/Window.hpp"

#include "AssetLibrary.hpp"
#include "EditorApp.hpp"
#include "EditorAssetLoader.hpp"
#include "EditorConstants.hpp"

int main(int argc, char** argv)
{
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

	// Memory

	Memory::InitializeMemorySystem();

	{
		Allocator* defaultAlloc = Memory::GetDefaultAllocator();
		AllocatorManager* allocManager = defaultAlloc->MakeNew<AllocatorManager>(defaultAlloc);

		// Virtual filesystem
		// Initial virtual mount points before editor project is loaded

		Allocator* filesystemAllocator = allocManager->CreateAllocatorScope("Filesystem", defaultAlloc);
		FilesystemVirtual filesystem(filesystemAllocator);

		using EditorConst = kokko::editor::EditorConstants;

		FilesystemVirtual::MountPoint mounts[] = {
			FilesystemVirtual::MountPoint{ StringRef(EditorConst::VirtualMountEngine), StringRef("engine/res") },
			FilesystemVirtual::MountPoint{ StringRef(EditorConst::VirtualMountEditor), StringRef("editor/res") }
		};
		filesystem.SetMountPoints(ArrayView(mounts));

		Allocator* appAllocator = allocManager->CreateAllocatorScope("EditorApp", defaultAlloc);
		kokko::editor::EditorApp editor(appAllocator, &filesystem);
		kokko::editor::AssetLibrary* assetLibrary = editor.GetAssetLibrary();
		kokko::editor::EditorAssetLoader assetLoader(appAllocator, &filesystem, assetLibrary);

		// TODO: Make sure EditorApp GPU resources are released before Engine is destroyed
		// 

		// Engine

		Engine engine(allocManager, &filesystem, &assetLoader);

		if (assetLibrary->ScanEngineAssets() == false)
		{
			instr.EndSession();

			res = -1;
		}
		else if (engine.Initialize())
		{
			editor.Initialize(&engine);

			engine.SetAppPointer(&editor);

			instr.EndSession();

			while (engine.GetMainWindow()->GetShouldClose() == false)
			{
				engine.StartFrame();
				editor.StartFrame();

				engine.UpdateWorld();

				// Because editor can change the state of the world and systems,
				// let's run those updates at the same part of the frame as other updates
				bool editorWantsToExit = false;
				editor.Update(engine.GetSettings(), editorWantsToExit);
				if (editorWantsToExit)
					engine.GetMainWindow()->SetShouldClose(true);

				engine.Render(editor.GetEditorCameraParameters(), editor.GetSceneViewFramebuffer());

				editor.EndFrame();
				engine.EndFrame();
			}
		}
		else
		{
			instr.EndSession();

			res = -1;
		}
	}

	Memory::DeinitializeMemorySystem();

	return res;
}
	