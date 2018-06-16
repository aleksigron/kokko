#include "Debug.hpp"

#include <cstdio>

#include "IncludeOpenGL.hpp"

#include "Engine.hpp"
#include "Time.hpp"
#include "Window.hpp"
#include "KeyboardInput.hpp"
#include "BitmapFont.hpp"

#include "DebugVectorRenderer.hpp"
#include "DebugTextRenderer.hpp"
#include "DebugConsole.hpp"
#include "DebugLog.hpp"
#include "DebugGraph.hpp"

Debug::Debug() :
	window(nullptr),
	mode(DebugMode::None),
	currentFrameRate(0.0),
	nextFrameRateUpdate(-1.0)
{
	vectorRenderer = new DebugVectorRenderer;
	textRenderer = new DebugTextRenderer;
	graph = new DebugGraph(vectorRenderer);
	console = new DebugConsole(textRenderer, vectorRenderer);
	log = new DebugLog(console);
}

Debug::~Debug()
{
	delete log;
	delete console;
	delete graph;
	delete textRenderer;
	delete vectorRenderer;
}

void Debug::SetWindow(Window* window)
{
	this->window = window;

	Vec2f frameSize = this->window->GetFrameBufferSize();
	float screenCoordScale = this->window->GetScreenCoordinateScale();

	textRenderer->SetFrameSize(frameSize);
	textRenderer->SetScaleFactor(screenCoordScale);

	Vec2f trScaledFrameSize = textRenderer->GetScaledFrameSize();
	int scaledLineHeight = textRenderer->GetFont()->GetLineHeight();
	float pixelLineHeight = scaledLineHeight * screenCoordScale;

	Rectangle logArea;
	logArea.position.x = 0.0f;
	logArea.position.y = scaledLineHeight;
	logArea.size.x = trScaledFrameSize.x;
	logArea.size.y = trScaledFrameSize.y - scaledLineHeight;
	console->SetDrawArea(logArea);

	Rectangle graphArea;
	graphArea.position.x = 0.0f;
	graphArea.position.y = pixelLineHeight;
	graphArea.size.x = frameSize.x;
	graphArea.size.y = frameSize.y - pixelLineHeight;
	graph->SetDrawArea(graphArea);
}

void Debug::Render()
{
	bool vsync = false;

	if (window != nullptr)
	{
		DebugMode oldMode = this->mode;

		KeyboardInput* keyboard = window->GetKeyboardInput();

		// Update mode
		if (keyboard->GetKeyDown(Key::Escape))
		{
			this->mode = DebugMode::None;
		}
		else if (keyboard->GetKeyDown(Key::F1))
		{
			this->mode = DebugMode::Console;
		}
		else if (keyboard->GetKeyDown(Key::F2))
		{
			this->mode = DebugMode::FrameTime;
		}

		vsync = window->GetSwapInterval() != 0;
		if (keyboard->GetKeyDown(Key::F8))
		{
			vsync = !vsync;
			window->SetSwapInterval(vsync ? 1 : 0);
		}

		if (oldMode != DebugMode::Console && this->mode == DebugMode::Console)
			console->RequestFocus();
		else if (oldMode == DebugMode::Console && this->mode != DebugMode::Console)
			console->ReleaseFocus();
	}

	char modeLogChar = (mode == DebugMode::Console) ? '*' : ' ';
	char modeTimeChar = (mode == DebugMode::FrameTime) ? '*' : ' ';
	char vsyncChar = vsync ? 'Y' : 'N';

	double now = Time::GetRunningTime();
	if (now > nextFrameRateUpdate)
	{
		currentFrameRate = 1.0 / graph->GetAverageOverLastSeconds(0.15);
		nextFrameRateUpdate = now + 0.15;
	}

	// Draw debug mode guide
	char buffer[128];
	const char* format = "Debug: [F1]Console%c [F2]FrameTime%c [F8]Vsync: %c, %.1f fps";
	snprintf(buffer, sizeof(buffer), format, modeLogChar, modeTimeChar, vsyncChar, currentFrameRate);
	textRenderer->AddText(StringRef(buffer), Vec2f(0.0f, 0.0f), true);

	// Add frame time to debug graph
	graph->AddDataPoint(Time::GetDeltaTime());
	graph->Update();

	// Draw mode content
	switch (this->mode)
	{
		case DebugMode::None:
			break;

		case DebugMode::Console:
			console->DrawToRenderers();
			break;

		case DebugMode::FrameTime:
			graph->DrawToVectorRenderer();
			break;
	}

	vectorRenderer->Render();

	// Draw debug texts
	textRenderer->Render();
}

void Debug::CheckOpenGlErrors()
{
	GLenum error;
	while ((error = glGetError()) != GL_NO_ERROR)
	{
		char buffer[32];
		sprintf(buffer, "glGetError() -> %u", error);

		log->Log(StringRef(buffer));
	}
}
