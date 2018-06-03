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
	mode(DebugMode::None)
{
	vectorRenderer = new DebugVectorRenderer;
	textRenderer = new DebugTextRenderer;
	graph = new DebugGraph(vectorRenderer);
	console = new DebugConsole(textRenderer);
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
		KeyboardInput* keyboard = window->GetKeyboardInput();

		// Update mode
		if (keyboard->GetKeyDown(Key::N_1))
		{
			this->mode = DebugMode::None;
		}
		else if (keyboard->GetKeyDown(Key::N_2))
		{
			this->mode = DebugMode::Console;
		}
		else if (keyboard->GetKeyDown(Key::N_3))
		{
			this->mode = DebugMode::FrameTime;
		}

		vsync = window->GetSwapInterval() != 0;
		if (keyboard->GetKeyDown(Key::N_0))
		{
			vsync = !vsync;
			window->SetSwapInterval(vsync ? 1 : 0);
		}
	}

	char modeNoneChar = (mode == DebugMode::None) ? '*' : ' ';
	char modeLogChar = (mode == DebugMode::Console) ? '*' : ' ';
	char modeTimeChar = (mode == DebugMode::FrameTime) ? '*' : ' ';
	const char* vsyncStr = vsync ? "On" : "Off";

	// Draw debug mode guide
	char buffer[128];
	sprintf(buffer, "Debug mode: [1]None%c [2]Log%c [3]FrameTime%c | [0]Vsync: %s", modeNoneChar, modeLogChar, modeTimeChar, vsyncStr);
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
			console->DrawToTextRenderer();
			break;

		case DebugMode::FrameTime:
			this->DrawFrameTimeStats();
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

void Debug::DrawFrameTimeStats()
{
	Engine* engine = Engine::GetInstance();

	float deltaTime = Time::GetDeltaTime();
	float fps = 1.0f / deltaTime;
	float ms = deltaTime * 1000.0f;

	char frameRateText[32];
	sprintf(frameRateText, "%.1f fps, %.1f ms", double(fps), double(ms));

	Vec2f position(0.0f, textRenderer->GetFont()->GetLineHeight());
	textRenderer->AddText(StringRef(frameRateText), position, true);

	graph->DrawToVectorRenderer();
}
