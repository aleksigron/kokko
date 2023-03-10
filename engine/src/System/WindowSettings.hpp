#pragma once

namespace kokko
{

struct WindowSettings
{
	bool verticalSync = true;
	bool visible = true;
	bool maximized = false;
	int width = 0;
	int height = 0;
	const char* title;
};

}
