#pragma once

class DebugVectorRenderer;
class DebugTextRenderer;
class DebugGraph;
class DebugCulling;
class DebugConsole;
class DebugLog;

class Window;
class Scene;

class Debug
{
private:
	DebugVectorRenderer* vectorRenderer;
	DebugTextRenderer* textRenderer;
	DebugGraph* graph;
	DebugCulling* culling;
	DebugConsole* console;
	DebugLog* log;

	Window* window;

	double currentFrameRate;
	double nextFrameRateUpdate;

	enum class DebugMode
	{
		None,
		Console,
		FrameTime,
		CullingPri,
		CullingSec
	}
	mode;

public:
	Debug();
	~Debug();

	void SetWindow(Window* window);
	
	void Render(Scene* scene);

	void CheckOpenGlErrors();

	DebugLog* GetLog() { return log; }
	DebugConsole* GetConsole() { return console; }
	DebugTextRenderer* GetTextRenderer() { return textRenderer; }
	DebugVectorRenderer* GetVectorRenderer() { return vectorRenderer; }
};
