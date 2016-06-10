#include "Debug.hpp"

#include "DebugLog.hpp"
#include "DebugTextRenderer.hpp"

Debug::Debug()
{
	log = new DebugLog;
	textRenderer = new DebugTextRenderer;
}

Debug::~Debug()
{
	delete textRenderer;
	delete log;
}
