#include "Engine.hpp"
#include "App.hpp"
#include "System/Window.hpp"

int main(void)
{
	Engine engine;

	if (engine.Initialize())
	{
		App app;

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