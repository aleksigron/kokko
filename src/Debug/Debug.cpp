#include "Debug/Debug.hpp"

#include <cassert>
#include <cstdio>

#include "System/IncludeOpenGL.hpp"

#include "Engine/Engine.hpp"
#include "System/Time.hpp"
#include "System/Window.hpp"
#include "Scene/Scene.hpp"
#include "System/InputManager.hpp"
#include "System/KeyboardInputView.hpp"
#include "Resources/BitmapFont.hpp"
#include "Rendering/Renderer.hpp"

#include "Debug/DebugVectorRenderer.hpp"
#include "Debug/DebugTextRenderer.hpp"
#include "Debug/DebugGraph.hpp"
#include "Debug/DebugCulling.hpp"
#include "Debug/DebugConsole.hpp"
#include "Debug/DebugLog.hpp"
#include "Debug/DebugMemoryStats.hpp"

Debug::Debug(Allocator* allocator) :
	allocator(allocator),
	window(nullptr),
	currentFrameRate(0.0),
	nextFrameRateUpdate(-1.0),
	mode(DebugMode::None)
{
	vectorRenderer = allocator->MakeNew<DebugVectorRenderer>();
	textRenderer = allocator->MakeNew<DebugTextRenderer>(allocator);
	graph = allocator->MakeNew<DebugGraph>(vectorRenderer);
	culling = allocator->MakeNew<DebugCulling>(textRenderer, vectorRenderer);
	console = allocator->MakeNew<DebugConsole>(textRenderer, vectorRenderer);
	log = allocator->MakeNew<DebugLog>(console);

	AllocatorManager* allocManager = Engine::GetInstance()->GetAllocatorManager();
	memoryStats = allocator->MakeNew<DebugMemoryStats>(allocManager, textRenderer);
}

Debug::~Debug()
{
	allocator->MakeDelete(memoryStats);
	allocator->MakeDelete(log);
	allocator->MakeDelete(console);
	allocator->MakeDelete(culling);
	allocator->MakeDelete(graph);
	allocator->MakeDelete(textRenderer);
	allocator->MakeDelete(vectorRenderer);
}

void Debug::SetWindow(Window* window)
{
	this->window = window;

	Vec2f frameSize = this->window->GetFrameBufferSize();
	float screenCoordScale = this->window->GetScreenCoordinateScale();

	textRenderer->SetFrameSize(frameSize);
	textRenderer->SetScaleFactor(screenCoordScale);

	float scaledLineHeight = 0;
	const BitmapFont* font = textRenderer->GetFont();

	if (font != nullptr)
		scaledLineHeight = static_cast<float>(font->GetLineHeight());

	float pixelLineHeight = scaledLineHeight * screenCoordScale;

	Vec2f trScaledFrameSize = textRenderer->GetScaledFrameSize();

	Rectanglef textArea;
	textArea.position.x = 0.0f;
	textArea.position.y = scaledLineHeight;
	textArea.size.x = trScaledFrameSize.x;
	textArea.size.y = trScaledFrameSize.y - scaledLineHeight;

	console->SetDrawArea(textArea);
	memoryStats->SetDrawArea(textArea);

	Rectanglef graphArea;
	graphArea.position.x = 0.0f;
	graphArea.position.y = pixelLineHeight;
	graphArea.size.x = frameSize.x;
	graphArea.size.y = frameSize.y - pixelLineHeight;
	graph->SetDrawArea(graphArea);

	culling->SetGuideTextPosition(Vec2f(0.0f, scaledLineHeight));
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
		else if (keyboard->GetKeyDown(Key::F4))
		{
			this->mode = DebugMode::MemoryStats;
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

	char memChar = (mode == DebugMode::MemoryStats) ? '*' : ' ';

	char vsyncChar = vsync ? 'Y' : 'N';

	double now = Time::GetRunningTime();
	if (now > nextFrameRateUpdate)
	{
		currentFrameRate = 1.0 / graph->GetAverageOverLastSeconds(0.15);
		nextFrameRateUpdate = now + 0.15;
	}

	// Draw debug mode guide
	char buffer[128];
	const char* format = "Debug: [F1]Console%c [F2]FrameTime%c [F3]Culling%c [F4]Memory%c [F8]Vsync: %c, %.1f fps";
	std::snprintf(buffer, sizeof(buffer), format, logChar, timeChar, cullChar, memChar, vsyncChar, currentFrameRate);
	textRenderer->AddText(StringRef(buffer), Vec2f(0.0f, 0.0f));

	// Add frame time to debug graph
	graph->AddDataPoint(Time::GetDeltaTime());
	graph->Update();

	if (mode == DebugMode::Console)
		console->UpdateAndDraw();

	if (mode == DebugMode::FrameTime)
		graph->DrawToVectorRenderer();

	if (cullingDebugEnable)
	{
		culling->UpdateAndDraw(scene);
	}

	if (mode == DebugMode::MemoryStats)
		memoryStats->UpdateAndDraw();

	vectorRenderer->Render(cullingDebugEnable ? culling->GetCamera() : scene->GetActiveCamera());
	textRenderer->Render();
}

void Debug::CheckOpenGlErrors()
{
	unsigned int errorCount = 0;

	GLenum error;
	while ((error = glGetError()) != GL_NO_ERROR)
	{
		const char* desc;

		switch (error)
		{
			case GL_INVALID_ENUM: desc = "GL_INVALID_ENUM"; break;
			case GL_INVALID_VALUE: desc = "GL_INVALID_VALUE"; break;
			case GL_INVALID_OPERATION: desc = "GL_INVALID_OPERATION"; break;
			case GL_OUT_OF_MEMORY: desc = "GL_OUT_OF_MEMORY"; break;
			default: desc = "<UNKNOWN>";
		}

		char buffer[128];
		int w = std::snprintf(buffer, sizeof(buffer), "glGetError(): 0x%04X, %s", error, desc);

		Engine::GetInstance()->GetDebug()->GetLog()->Log(StringRef(buffer, w));

		++errorCount;
	}

	assert(errorCount == 0);
}
