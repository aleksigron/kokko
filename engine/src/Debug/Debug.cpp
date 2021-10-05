#include "Debug/Debug.hpp"

#include <cassert>
#include <cstdio>

#include "Core/Core.hpp"

#include "Debug/DebugVectorRenderer.hpp"
#include "Debug/DebugTextRenderer.hpp"
#include "Debug/DebugGraph.hpp"
#include "Debug/DebugCulling.hpp"
#include "Debug/DebugConsole.hpp"
#include "Debug/DebugLog.hpp"
#include "Debug/DebugMemoryStats.hpp"
#include "Debug/Instrumentation.hpp"
#include "Debug/Log.hpp"

#include "Engine/World.hpp"

#include "Graphics/Scene.hpp"

#include "Math/Rectangle.hpp"

#include "Rendering/Framebuffer.hpp"
#include "Rendering/Renderer.hpp"
#include "Rendering/RenderDevice.hpp"

#include "Resources/BitmapFont.hpp"
#include "Resources/ShaderManager.hpp"

#include "System/InputManager.hpp"
#include "System/InputView.hpp"
#include "System/Time.hpp"
#include "System/Window.hpp"

static void RenderDebugCallback(const RenderDevice::DebugMessage& message)
{
	if (message.severity != RenderDebugSeverity::Notification)
		Log::Log(LogLevel::Warning, FMT_STRING("{}"), message.message.str);
}

Debug::Debug(
	Allocator* allocator,
	AllocatorManager* allocManager,
	Window* window,
	RenderDevice* renderDevice,
	Filesystem* filesystem) :
	allocator(allocator),
	renderDevice(renderDevice),
	window(nullptr),
	profileInProgress(false),
	profileStarted(false),
	endProfileOnFrame(0),
	currentFrameTime(0.0),
	nextFrameRateUpdate(-1.0),
	mode(DebugMode::None)
{
	vectorRenderer = allocator->MakeNew<DebugVectorRenderer>(allocator, renderDevice);
	textRenderer = allocator->MakeNew<DebugTextRenderer>(allocator, renderDevice, filesystem);
	graph = allocator->MakeNew<DebugGraph>(allocator, vectorRenderer);
	culling = allocator->MakeNew<DebugCulling>(textRenderer, vectorRenderer);
	console = allocator->MakeNew<DebugConsole>(allocator, window, textRenderer, vectorRenderer);
	log = allocator->MakeNew<DebugLog>(allocator, console);

	// Set up log instance
	Log::SetLogInstance(log);

	memoryStats = allocator->MakeNew<DebugMemoryStats>(allocManager, textRenderer);
}

Debug::~Debug()
{
	allocator->MakeDelete(memoryStats);

	// Clear log instance in LogHelper
	Log::SetLogInstance(nullptr);

	allocator->MakeDelete(log);
	allocator->MakeDelete(console);
	allocator->MakeDelete(culling);
	allocator->MakeDelete(graph);
	allocator->MakeDelete(textRenderer);
	allocator->MakeDelete(vectorRenderer);
}

void Debug::Initialize(Window* window, MeshManager* meshManager,
	ShaderManager* shaderManager, TextureManager* textureManager)
{
	KOKKO_PROFILE_FUNCTION();

	{
		KOKKO_PROFILE_SCOPE("void RenderDevice::SetDebugMessageCallback()");
		renderDevice->SetDebugMessageCallback(RenderDebugCallback);
	}

	this->window = window;

	textRenderer->Initialize(shaderManager, meshManager, textureManager);
	vectorRenderer->Initialize(meshManager, shaderManager);
}

void Debug::Deinitialize()
{
	vectorRenderer->Deinitialize();
}

void Debug::Render(World* world, const Framebuffer& framebuffer, const Optional<CameraParameters>& editorCamera)
{
	KOKKO_PROFILE_FUNCTION();

	Vec2i framebufferSize(framebuffer.GetWidth(), framebuffer.GetHeight());
	Vec2f frameSizef = framebufferSize.As<float>();
	float screenCoordScale = this->window->GetScreenCoordinateScale();

	textRenderer->SetFrameSize(frameSizef);
	textRenderer->SetScaleFactor(screenCoordScale);

	float scaledLineHeight = 0;
	const BitmapFont* font = textRenderer->GetFont();

	assert(font != nullptr);

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
	graphArea.size.x = frameSizef.x;
	graphArea.size.y = frameSizef.y - pixelLineHeight;
	graph->SetDrawArea(graphArea);

	culling->SetGuideTextPosition(Vec2f(0.0f, scaledLineHeight));

	bool vsync = false;

	if (window != nullptr)
	{
		DebugMode oldMode = this->mode;

		InputView* input = window->GetInputManager()->GetGameInputView();

		// Update mode
		if (input->GetKeyDown(KeyCode::Escape))
		{
			this->mode = DebugMode::None;
		}
		else if (input->GetKeyDown(KeyCode::F1))
		{
			this->mode = DebugMode::Console;
		}
		else if (input->GetKeyDown(KeyCode::F2))
		{
			this->mode = DebugMode::FrameTime;
		}
		else if (input->GetKeyDown(KeyCode::F3))
		{
			this->mode = DebugMode::Culling;
		}
		else if (input->GetKeyDown(KeyCode::F4))
		{
			this->mode = DebugMode::MemoryStats;
		}

		if (input->GetKeyDown(KeyCode::F7) && profileInProgress == false)
		{
			RequestBeginProfileSession();
		}

		// Check vsync switching

		vsync = window->GetSwapInterval() != 0;
		if (input->GetKeyDown(KeyCode::F8))
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

		Renderer* renderer = world->GetRenderer();

		if (oldMode != DebugMode::Culling && this->mode == DebugMode::Culling)
		{
			// Disable main camera controller
			culling->SetLockCullingCamera(true);
			renderer->SetLockCullingCamera(true);
		}
		else if (oldMode == DebugMode::Culling && this->mode != DebugMode::Culling)
		{
			// Enable main camera controller
			culling->SetLockCullingCamera(false);
			renderer->SetLockCullingCamera(false);
		}
	}

	char logChar = (mode == DebugMode::Console) ? '*' : ' ';
	char timeChar = (mode == DebugMode::FrameTime) ? '*' : ' ';
	char cullChar = (mode == DebugMode::Culling) ? '*' : ' ';
	char memChar = (mode == DebugMode::MemoryStats) ? '*' : ' ';

	char vsyncChar = vsync ? 'Y' : 'N';

	double now = Time::GetRunningTime();
	if (now > nextFrameRateUpdate)
	{
		currentFrameTime = graph->GetAverageOverLastSeconds(0.15);
		nextFrameRateUpdate = now + 0.15;
	}

	unsigned int errs = console->GetTotalErrorCount();
	unsigned int wrns = console->GetTotalWarningCount();

	int glyphWidth = font->GetGlyphWidth();
	int lineHeight = font->GetLineHeight();

	if (errs > 0)
	{
		Rectanglef rect;
		rect.position.x = 1.0f;
		rect.position.y = 0.0f;
		rect.size.x = static_cast<float>(6 * glyphWidth) - 1.0f;
		rect.size.y = static_cast<float>(lineHeight);

		Color red(1.0f, 0.0f, 0.0f);
		vectorRenderer->DrawRectangleScreen(rect, red);
	}

	if (wrns > 0)
	{
		Rectanglef rect;
		rect.position.x = static_cast<float>(7 * glyphWidth) + 1.0f;
		rect.position.y = 0.0f;
		rect.size.x = static_cast<float>(6 * glyphWidth) - 1.0f;
		rect.size.y = static_cast<float>(lineHeight);

		Color yellow(1.0f, 1.0f, 0.0f);
		vectorRenderer->DrawRectangleScreen(rect, yellow);
	}

	double currentFrameRate = 1.0 / currentFrameTime;

	// Draw debug mode guide
	char buffer[128];
	const char* format = "E: %-3u W: %-3u [F1]Console%c [F2]FrameTime%c [F3]Culling%c [F4]Memory%c [F7] Start profile  [F8]Vsync: %c, %.1f fps";
	std::snprintf(buffer, sizeof(buffer), format, errs, wrns, logChar, timeChar, cullChar, memChar, vsyncChar, currentFrameRate);
	textRenderer->AddText(StringRef(buffer), Vec2f(0.0f, 0.0f));

	// Add frame time to debug graph
	graph->AddDataPoint(Time::GetDeltaTime());
	graph->Update();

	if (mode == DebugMode::Console)
		console->UpdateAndDraw();

	if (mode == DebugMode::FrameTime)
		graph->DrawToVectorRenderer();

	if (mode == DebugMode::Culling)
		culling->UpdateAndDraw(world);

	if (mode == DebugMode::MemoryStats)
		memoryStats->UpdateAndDraw();

	ViewRectangle viewport;
	viewport.position = Vec2i();
	viewport.size = framebufferSize;

	vectorRenderer->Render(world, viewport, editorCamera);
	textRenderer->Render();
}

void Debug::RequestBeginProfileSession()
{
	if (profileInProgress == false)
	{
		profileInProgress = true;
		profileStarted = false;
		endProfileOnFrame = Time::GetFrameNumber() + 100;
	}
}

bool Debug::ShouldBeginProfileSession() const
{
	return profileInProgress && profileStarted == false;
}

bool Debug::ShouldEndProfileSession()
{
	bool ended = profileInProgress && Time::GetFrameNumber() >= endProfileOnFrame;

	if (ended)
	{
		profileInProgress = false;
		endProfileOnFrame = 0;
	}

	return ended;
}
