#pragma once

class Allocator;
class RenderDevice;
class MeshManager;
class ShaderManager;
class Scene;
class Window;

class DebugVectorRenderer;
class DebugTextRenderer;
class DebugGraph;
class DebugCulling;
class DebugConsole;
class DebugLog;
class DebugMemoryStats;

class Debug
{
private:
	Allocator* allocator;

	DebugVectorRenderer* vectorRenderer;
	DebugTextRenderer* textRenderer;
	DebugGraph* graph;
	DebugCulling* culling;
	DebugConsole* console;
	DebugLog* log;
	DebugMemoryStats* memoryStats;

	Window* window;

	double currentFrameRate;
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
	Debug(Allocator* allocator, RenderDevice* renderDevice);
	~Debug();

	void Initialize(Window* window, MeshManager* meshManager, ShaderManager* shaderManager);
	void Deinitialize();
	
	void Render(Scene* scene);

	static void CheckOpenGlErrors();

	DebugLog* GetLog() { return log; }
	DebugConsole* GetConsole() { return console; }
	DebugTextRenderer* GetTextRenderer() { return textRenderer; }
	DebugVectorRenderer* GetVectorRenderer() { return vectorRenderer; }
};
