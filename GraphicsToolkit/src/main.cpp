#include "App.hpp"

int main(void)
{
	App app;
	
	if (app.Initialize())
	{
		while (app.HasRequestedQuit() == false)
		{
			app.Update();
		}
	}
	else
		return -1;
	
	return 0;
}