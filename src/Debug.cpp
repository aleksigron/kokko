#include "Debug.hpp"

#include "DebugTextRenderer.hpp"
#include "DebugLogView.hpp"
#include "DebugLog.hpp"

Debug::Debug()
{
	textRenderer = new DebugTextRenderer;
	logView = new DebugLogView(textRenderer);
	log = new DebugLog(logView);
}

Debug::~Debug()
{
	delete log;
	delete logView;
	delete textRenderer;
}
