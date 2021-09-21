#include "EditorApp.hpp"

#include "doctest/doctest.h"

#include "Debug/Instrumentation.hpp"

#include "Engine/Engine.hpp"

#include "Memory/Memory.hpp"
#include "Memory/AllocatorManager.hpp"

#include "Rendering/CameraParameters.hpp"

#include "System/Window.hpp"

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

	Engine engine;

	if (engine.Initialize())
	{
		AllocatorManager* am = engine.GetAllocatorManager();
		Allocator* defaultAlloc = Memory::GetDefaultAllocator();
		Allocator* appAllocator = am->CreateAllocatorScope("EditorApp", defaultAlloc);

		EditorApp editor(appAllocator);
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
			editor.Update(engine.GetSettings(), engine.GetWorld(), editorWantsToExit);
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
		return -1;
	}

	return res;
}
	