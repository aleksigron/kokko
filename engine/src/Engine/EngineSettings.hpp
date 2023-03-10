#pragma once

#include "Rendering/RenderDebugSettings.hpp"

namespace kokko
{

struct EngineSettings
{
	bool verticalSync = true;
	bool enableDebugTools = true;
	
	RenderDebugSettings renderDebug;
};

}
