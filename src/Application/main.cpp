#include "Application/App.hpp"
#include "Engine/Engine.hpp"
#include "Memory/Memory.hpp"
#include "Memory/AllocatorManager.hpp"
#include "System/Window.hpp"

int main(void)
{
	Engine engine;

	if (engine.Initialize())
	{
		AllocatorManager* am = engine.GetAllocatorManager();
		Allocator* defaultAlloc = Memory::GetDefaultAllocator();
		Allocator* appAllocator = am->CreateAllocatorScope("Application", defaultAlloc);

		App app(&engine, appAllocator);

		app.Initialize();

		while (engine.GetMainWindow()->ShouldClose() == false)
		{
			engine.Update();
			app.Update();
		}
	}
	else
		return -1;

	return 0;
}