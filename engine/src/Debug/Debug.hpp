#pragma once

#include "Core/Optional.hpp"
#include "Core/StringView.hpp"

#include "Math/Vec2.hpp"

class Allocator;
class AllocatorManager;
class Renderer;

class DebugTextRenderer;
class DebugGraph;
class DebugCulling;
class DebugConsole;
class DebugMemoryStats;

struct ViewRectangle;
struct CameraParameters;

namespace kokko
{
class DebugVectorRenderer;
class Filesystem;
class MeshManager;
class ShaderManager;
class TextureManager;
class Window;
class World;

namespace render
{
class CommandEncoder;
class Device;
class Framebuffer;
}
}

class Debug
{
private:
	static Debug* singletonInstance;

	Allocator* allocator;
	kokko::render::Device* renderDevice;

	kokko::DebugVectorRenderer* vectorRenderer;
	DebugTextRenderer* textRenderer;
	DebugGraph* graph;
	DebugCulling* culling;
	DebugConsole* console;
	DebugMemoryStats* memoryStats;

	kokko::Window* window;

	bool profileInProgress;
	bool profileStarted;
	unsigned int endProfileOnFrame;

	double currentFrameTime;
	double nextFrameRateUpdate;

	enum class DebugMode
	{
		None,
		Console,
		FrameTime,
		Culling,
		MemoryStats
	}
	mode;

public:
	Debug(Allocator* allocator, AllocatorManager* allocManager, kokko::render::Device* renderDevice,
        kokko::Filesystem* filesystem);
	~Debug();

	static Debug* Get() { return singletonInstance; }

	bool Initialize(kokko::Window* window, kokko::MeshManager* meshManager,
		kokko::ShaderManager* shaderManager, kokko::TextureManager* textureManager);
	void Deinitialize();

	void Render(
		kokko::render::CommandEncoder* encoder,
		kokko::World* world,
		const kokko::render::Framebuffer& framebuffer,
		const Optional<CameraParameters>& editorCamera);

	DebugConsole* GetConsole() { return console; }
	DebugTextRenderer* GetTextRenderer() { return textRenderer; }
	kokko::DebugVectorRenderer* GetVectorRenderer() { return vectorRenderer; }

	void RequestBeginProfileSession();

	bool ShouldBeginProfileSession() const;
	bool ShouldEndProfileSession();
};
