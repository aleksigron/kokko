#include "Debug/Debug.hpp"

#include <cassert>
#include <cstdio>

#include "Core/Core.hpp"

#include "Debug/DebugVectorRenderer.hpp"
#include "Debug/DebugTextRenderer.hpp"
#include "Debug/DebugGraph.hpp"
#include "Debug/DebugCulling.hpp"
#include "Debug/DebugMemoryStats.hpp"
#include "Debug/Instrumentation.hpp"

#include "Engine/World.hpp"

#include "Graphics/Scene.hpp"

#include "Math/Rectangle.hpp"

#include "Platform/Window.hpp"

#include "Rendering/Framebuffer.hpp"
#include "Rendering/Renderer.hpp"
#include "Rendering/RenderDevice.hpp"
#include "Rendering/RenderTypes.hpp"

#include "Resources/BitmapFont.hpp"
#include "Resources/ShaderManager.hpp"

#include "System/InputManager.hpp"
#include "System/InputView.hpp"
#include "System/Log.hpp"
#include "System/Time.hpp"

Debug* Debug::singletonInstance = nullptr;

static void RenderDebugCallback(const kokko::render::Device::DebugMessage& message)
{
	if (message.severity != RenderDebugSeverity::Notification)
	{
		LogLevel level = message.type == RenderDebugType::Error ? LogLevel::Error : LogLevel::Warning;
		kokko::Log::Log(level, FMT_STRING("{}"), message.message.str);
	}
}

Debug::Debug(
	Allocator* allocator,
	AllocatorManager* allocManager,
	kokko::render::Device* renderDevice,
	kokko::Filesystem* filesystem) :
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
	vectorRenderer = kokko::MakeUnique<kokko::DebugVectorRenderer>(allocator, allocator, renderDevice);
	textRenderer = kokko::MakeUnique<kokko::DebugTextRenderer>(allocator, allocator, renderDevice, filesystem);
	graph = kokko::MakeUnique<DebugGraph>(allocator, allocator, vectorRenderer.Get());
	culling = kokko::MakeUnique<kokko::DebugCulling>(allocator, textRenderer.Get(), vectorRenderer.Get());
	memoryStats = kokko::MakeUnique<DebugMemoryStats>(allocator, allocManager, textRenderer.Get());

	singletonInstance = this;
}

Debug::~Debug()
{
	singletonInstance = nullptr;
}

bool Debug::Initialize(kokko::Window* window, kokko::ModelManager* modelManager,
	kokko::ShaderManager* shaderManager, kokko::TextureManager* textureManager)
{
	KOKKO_PROFILE_FUNCTION();

	{
		KOKKO_PROFILE_SCOPE("void RenderDevice::SetDebugMessageCallback()");
		renderDevice->SetDebugMessageCallback(RenderDebugCallback);
	}

	this->window = window;

	if (textRenderer->Initialize(shaderManager, modelManager, textureManager) == false)
		return false;

	vectorRenderer->Initialize(modelManager, shaderManager);

	return true;
}

void Debug::Deinitialize()
{
	vectorRenderer->Deinitialize();
}

void Debug::Render(kokko::render::CommandEncoder* encoder, kokko::World* world,
	const kokko::render::Framebuffer& framebuffer, const Optional<CameraParameters>& editorCamera)
{
	KOKKO_PROFILE_FUNCTION();

#ifdef KOKKO_USE_METAL
    return;
#endif

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

        // TODO: Fix debug key shortcuts
		//vsync = window->GetSwapInterval() != 0;
		if (input->GetKeyDown(KeyCode::F8))
		{
			vsync = !vsync;
			//window->SetSwapInterval(vsync ? 1 : 0);
		}

		// Check culling camera controller switching

		kokko::Renderer* renderer = world->GetRenderer();

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

	double currentFrameRate = 1.0 / currentFrameTime;

	// Draw debug mode guide
	char buffer[128];
	const char* format = "[F2]FrameTime%c [F3]Culling%c [F4]Memory%c [F7] Start profile, %.1f fps";
	std::snprintf(buffer, sizeof(buffer), format, timeChar, cullChar, memChar, currentFrameRate);
	textRenderer->AddText(kokko::ConstStringView(buffer), Vec2f(0.0f, 0.0f));

	// Add frame time to debug graph
	graph->AddDataPoint(Time::GetDeltaTime());
	graph->Update();

	if (mode == DebugMode::FrameTime)
		graph->DrawToVectorRenderer();

	if (mode == DebugMode::Culling)
		culling->UpdateAndDraw(world);

	if (mode == DebugMode::MemoryStats)
		memoryStats->UpdateAndDraw();

	ViewRectangle viewport;
	viewport.position = Vec2i();
	viewport.size = framebufferSize;

	vectorRenderer->Render(encoder, world, viewport, editorCamera);
	textRenderer->Render(encoder);
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
