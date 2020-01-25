#pragma once

class Allocator;

class DebugVectorRenderer;
class DebugTextRenderer;
class DebugGraph;
class DebugCulling;
class DebugConsole;
class DebugLog;
class DebugMemoryStats;

class Window;
class Scene;

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
		CullingPri,
		CullingSec,
		MemoryStats
	}
	mode;

public:
	Debug(Allocator* allocator);
	~Debug();

	void SetWindow(Window* window);
	
	void Render(Scene* scene);

	static void CheckOpenGlErrors();

	DebugLog* GetLog() { return log; }
	DebugConsole* GetConsole() { return console; }
	DebugTextRenderer* GetTextRenderer() { return textRenderer; }
	DebugVectorRenderer* GetVectorRenderer() { return vectorRenderer; }
};
