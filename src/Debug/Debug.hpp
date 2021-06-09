#pragma once

#include "Core/Optional.hpp"

class Allocator;
class AllocatorManager;
class CameraSystem;
class RenderDevice;
class MeshManager;
class ShaderManager;
class Scene;
class Window;
class Renderer;
class SceneManager;

class DebugVectorRenderer;
class DebugTextRenderer;
class DebugGraph;
class DebugCulling;
class DebugConsole;
class DebugLog;
class DebugMemoryStats;

struct ViewRectangle;
struct CameraParameters;

class Debug
{
private:
	Allocator* allocator;
	RenderDevice* renderDevice;

	DebugVectorRenderer* vectorRenderer;
	DebugTextRenderer* textRenderer;
	DebugGraph* graph;
	DebugCulling* culling;
	DebugConsole* console;
	DebugLog* log;
	DebugMemoryStats* memoryStats;

	Window* window;

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
	Debug(Allocator* allocator, AllocatorManager* allocManager,
		Window* window, RenderDevice* renderDevice);
	~Debug();

	void Initialize(Window* window, Renderer* renderer, CameraSystem* cameraSystem,
		MeshManager* meshManager, ShaderManager* shaderManager, Scene* scene);
	void Deinitialize();
	
	void Render(const ViewRectangle& viewport, const Optional<CameraParameters>& editorCamera);

	DebugLog* GetLog() { return log; }
	DebugConsole* GetConsole() { return console; }
	DebugTextRenderer* GetTextRenderer() { return textRenderer; }
	DebugVectorRenderer* GetVectorRenderer() { return vectorRenderer; }

	bool ShouldBeginProfileSession() const;
	bool ShouldEndProfileSession();
};
