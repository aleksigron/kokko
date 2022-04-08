#pragma once

#include "Rendering/RenderDebugSettings.hpp"

namespace kokko
{

struct EngineSettings
{
	bool verticalSync = true;
	
	RenderDebugSettings renderDebug;
};

}
