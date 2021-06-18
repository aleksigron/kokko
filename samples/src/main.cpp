#include "App.hpp"

#include "doctest/doctest.h"

#include "Debug/Instrumentation.hpp"

#include "Engine/Engine.hpp"

#include "Memory/Memory.hpp"
#include "Memory/AllocatorManager.hpp"

#include "System/Window.hpp"

int main(int argc, char** argv)
{
	doctest::Context ctx;
	ctx.setOption("abort-after", 5);
	ctx.applyCommandLine(argc, argv);
	ctx.setOption("no-breaks", true);
	int res = ctx.run();
	if (ctx.shouldExit())
		return res;

	Instrumentation& instr = Instrumentation::Get();
	instr.BeginSession("startup_trace.json");

	Engine engine;

	if (engine.Initialize())
	{
		AllocatorManager* am = engine.GetAllocatorManager();
		Allocator* defaultAlloc = Memory::GetDefaultAllocator();
		Allocator* appAllocator = am->CreateAllocatorScope("Application", defaultAlloc);

		App app(&engine, appAllocator);
		engine.SetAppPointer(&app);

		app.Initialize();

		instr.EndSession();

		while (engine.GetMainWindow()->GetShouldClose() == false)
		{
			engine.FrameStart();
			engine.Update();
			app.Update();
		}
	}
	else
	{
		instr.EndSession();
		return -1;
	}

	return res;
}
