#include "Debug.hpp"

#include <cstdio>

#include "IncludeOpenGL.hpp"

#include "Engine.hpp"
#include "Time.hpp"
#include "Window.hpp"
#include "Scene.hpp"
#include "InputManager.hpp"
#include "KeyboardInputView.hpp"
#include "BitmapFont.hpp"
#include "Renderer.hpp"

#include "DebugVectorRenderer.hpp"
#include "DebugTextRenderer.hpp"
#include "DebugGraph.hpp"
#include "DebugCulling.hpp"
#include "DebugConsole.hpp"
#include "DebugLog.hpp"

Debug::Debug() :
	window(nullptr),
	currentFrameRate(0.0),
	nextFrameRateUpdate(-1.0),
	mode(DebugMode::None)
{
	vectorRenderer = new DebugVectorRenderer;
	textRenderer = new DebugTextRenderer;
	graph = new DebugGraph(vectorRenderer);
	culling = new DebugCulling(vectorRenderer);
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

	int scaledLineHeight = 0;
	const BitmapFont* font = textRenderer->GetFont();

	if (font != nullptr)
		scaledLineHeight = font->GetLineHeight();

	float pixelLineHeight = scaledLineHeight * screenCoordScale;

	Vec2f trScaledFrameSize = textRenderer->GetScaledFrameSize();
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

void Debug::Render(Scene* scene)
{
	bool vsync = false;

	if (window != nullptr)
	{
		DebugMode oldMode = this->mode;

		KeyboardInputView* keyboard = window->GetInputManager()->GetKeyboardInputView();

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
		else if (keyboard->GetKeyDown(Key::F3))
		{
			if (oldMode == DebugMode::CullingPri)
				this->mode = DebugMode::CullingSec;
			else
				this->mode = DebugMode::CullingPri;
		}

		// Check vsync switching

		vsync = window->GetSwapInterval() != 0;
		if (keyboard->GetKeyDown(Key::F8))
		{
			vsync = !vsync;
			window->SetSwapInterval(vsync ? 1 : 0);
		}

		// Check console switching

		if (oldMode != DebugMode::Console && this->mode == DebugMode::Console)
			console->RequestFocus();
		else if (oldMode == DebugMode::Console && this->mode != DebugMode::Console)
			console->ReleaseFocus();

		// Check culling camera controller switching

		if (oldMode != DebugMode::CullingSec && this->mode == DebugMode::CullingSec)
		{
			// Disable main camera controller
			culling->SetControlledCamera(true);
		}
		else if (oldMode == DebugMode::CullingSec && this->mode != DebugMode::CullingSec)
		{
			// Enable main camera controller
			culling->SetControlledCamera(false);
		}

		// Check culling override camera switching

		if ((oldMode != DebugMode::CullingSec && oldMode != DebugMode::CullingPri) &&
			(this->mode == DebugMode::CullingSec || this->mode == DebugMode::CullingPri))
		{
			// Enable debug camera
			culling->EnableOverrideCamera(true);
		}
		else if ((oldMode == DebugMode::CullingSec || oldMode == DebugMode::CullingPri) &&
			(this->mode != DebugMode::CullingSec && this->mode != DebugMode::CullingPri))
		{
			// Disable debug camera
			culling->EnableOverrideCamera(false);
		}

	}

	char logChar = (mode == DebugMode::Console) ? '*' : ' ';
	char timeChar = (mode == DebugMode::FrameTime) ? '*' : ' ';

	bool cullingDebugEnable = mode == DebugMode::CullingPri || mode == DebugMode::CullingSec;
	char cullChar = cullingDebugEnable ? '*' : ' ';

	char vsyncChar = vsync ? 'Y' : 'N';

	double now = Time::GetRunningTime();
	if (now > nextFrameRateUpdate)
	{
		currentFrameRate = 1.0 / graph->GetAverageOverLastSeconds(0.15);
		nextFrameRateUpdate = now + 0.15;
	}

	// Draw debug mode guide
	char buffer[128];
	const char* format = "Debug: [F1]Console%c [F2]FrameTime%c [F3]Culling%c [F8]Vsync: %c, %.1f fps";
	std::snprintf(buffer, sizeof(buffer), format, logChar, timeChar, cullChar, vsyncChar, currentFrameRate);
	textRenderer->AddText(StringRef(buffer), Vec2f(0.0f, 0.0f));

	// Add frame time to debug graph
	graph->AddDataPoint(Time::GetDeltaTime());
	graph->Update();

	// Draw mode content
	switch (this->mode)
	{
		case DebugMode::None:
			break;

		case DebugMode::Console:
			console->UpdateAndDraw();
			break;

		case DebugMode::FrameTime:
			graph->DrawToVectorRenderer();
			break;

		case DebugMode::CullingPri:
			culling->UpdateAndDraw(scene);
			break;

		case DebugMode::CullingSec:
			culling->UpdateAndDraw(scene);
			break;
	}

	vectorRenderer->Render(cullingDebugEnable ? culling->GetCamera() : scene->GetActiveCamera());
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
