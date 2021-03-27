#include "Application/App.hpp"

#include "Debug/Instrumentation.hpp"

#include "Engine/Engine.hpp"

#include "Memory/Memory.hpp"
#include "Memory/AllocatorManager.hpp"

#include "System/Window.hpp"

int main(void)
{
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
		instr.BeginSession("runtime_trace.json");

		while (engine.GetMainWindow()->ShouldClose() == false)
		{
			engine.Update();
			app.Update();
		}

		instr.EndSession();
	}
	else
		return -1;

	return 0;
}
