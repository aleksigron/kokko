#pragma once

#include "Core/Optional.hpp"
#include "Core/StringView.hpp"
#include "Core/UniquePtr.hpp"

#include "Math/Vec2.hpp"

class Allocator;
class AllocatorManager;
class Renderer;

class DebugTextRenderer;
class DebugGraph;
class DebugCulling;
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

	kokko::UniquePtr<kokko::DebugVectorRenderer> vectorRenderer;
	kokko::UniquePtr<DebugTextRenderer> textRenderer;
	kokko::UniquePtr<DebugGraph> graph;
	kokko::UniquePtr<DebugCulling> culling;
	kokko::UniquePtr<DebugMemoryStats> memoryStats;

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

	DebugTextRenderer* GetTextRenderer() { return textRenderer.Get(); }
	kokko::DebugVectorRenderer* GetVectorRenderer() { return vectorRenderer.Get(); }

	void RequestBeginProfileSession();

	bool ShouldBeginProfileSession() const;
	bool ShouldEndProfileSession();
};
