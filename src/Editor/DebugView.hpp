#pragma once

struct EditorWindowInfo;

class Debug;

class DebugView
{
public:
	DebugView();

	void Initialize(Debug* debug);

	void Draw(EditorWindowInfo& windowInfo);

private:
	Debug* debug;
};
