#include "Debug.hpp"

#include <cstdio>

#include "IncludeOpenGL.hpp"

#include "Engine.hpp"
#include "Time.hpp"
#include "DebugVectorRenderer.hpp"
#include "DebugTextRenderer.hpp"
#include "DebugLogView.hpp"
#include "DebugLog.hpp"
#include "BitmapFont.hpp"

#include "Window.hpp"
#include "KeyboardInput.hpp"

Debug::Debug() :
	window(nullptr),
	mode(DebugMode::None)
{
	vectorRenderer = new DebugVectorRenderer;
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

void Debug::UpdateLogViewDrawArea()
{
	Vec2f size = textRenderer->GetScaledFrameSize();
	int lineHeight = textRenderer->GetFont()->GetLineHeight();

	Rectangle logArea;
	logArea.position.x = 0.0f;
	logArea.position.y = lineHeight;
	logArea.size.x = size.x;
	logArea.size.y = size.y - lineHeight;

	logView->SetDrawArea(logArea);
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
			this->mode = DebugMode::LogView;
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
	char modeLogChar = (mode == DebugMode::LogView) ? '*' : ' ';
	char modeTimeChar = (mode == DebugMode::FrameTime) ? '*' : ' ';
	const char* vsyncStr = vsync ? "On" : "Off";

	// Draw debug mode guide
	char buffer[128];
	sprintf(buffer, "Debug mode: [1]None%c [2]Log%c [3]FrameTime%c | [0]Vsync: %s", modeNoneChar, modeLogChar, modeTimeChar, vsyncStr);
	textRenderer->AddText(StringRef(buffer), Vec2f(0.0f, 0.0f), true);

	vectorRenderer->Render();

	// Draw mode content
	switch (this->mode)
	{
		case DebugMode::None:
			break;

		case DebugMode::LogView:
			logView->DrawToTextRenderer();
			break;

		case DebugMode::FrameTime:
			this->DrawFrameTimeStats();
			break;
	}

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

	Time* time = engine->GetTime();
	float deltaTime = time->GetDeltaTime();
	float fps = 1.0f / deltaTime;
	float ms = deltaTime * 1000.0f;

	char frameRateText[32];
	sprintf(frameRateText, "%.1f fps, %.1f ms", double(fps), double(ms));

	Vec2f position(0.0f, textRenderer->GetFont()->GetLineHeight());
	textRenderer->AddText(StringRef(frameRateText), position, true);
}
