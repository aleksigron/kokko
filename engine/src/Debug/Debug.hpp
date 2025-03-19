#pragma once

#include "Core/Optional.hpp"
#include "Core/StringView.hpp"
#include "Core/UniquePtr.hpp"

#include "Math/Vec2.hpp"

namespace kokko
{

class Allocator;
class AllocatorManager;
class DebugGraph;
class DebugMemoryStats;
class DebugCulling;
class DebugTextRenderer;
class DebugVectorRenderer;
class Filesystem;
class ModelManager;
class Renderer;
class ShaderManager;
class TextureManager;
class Window;
class World;

struct ViewRectangle;
struct CameraParameters;

namespace render
{
class CommandEncoder;
class Device;
class Framebuffer;
}

class Debug
{
private:
	static Debug* singletonInstance;

	Allocator* allocator;
	kokko::render::Device* renderDevice;

	kokko::UniquePtr<kokko::DebugVectorRenderer> vectorRenderer;
	kokko::UniquePtr<kokko::DebugTextRenderer> textRenderer;
	kokko::UniquePtr<DebugGraph> graph;
	kokko::UniquePtr<kokko::DebugCulling> culling;
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

	bool Initialize(kokko::Window* window, kokko::ModelManager* modelManager,
		kokko::ShaderManager* shaderManager, kokko::TextureManager* textureManager);
	void Deinitialize();

	void Render(
		kokko::render::CommandEncoder* encoder,
		kokko::World* world,
		const kokko::render::Framebuffer& framebuffer,
		const Optional<CameraParameters>& editorCamera);

	kokko::DebugTextRenderer* GetTextRenderer() { return textRenderer.Get(); }
	kokko::DebugVectorRenderer* GetVectorRenderer() { return vectorRenderer.Get(); }

	void RequestBeginProfileSession();

	bool ShouldBeginProfileSession() const;
	bool ShouldEndProfileSession();
};

} // namespace kokko
