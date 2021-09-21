#pragma once

struct EditorWindowInfo;
struct EngineSettings;

class Debug;

class DebugView
{
public:
	DebugView();

	void Initialize(Debug* debug);

	void Draw(EditorWindowInfo& windowInfo, EngineSettings* engineSettings);

private:
	Debug* debug;
};
