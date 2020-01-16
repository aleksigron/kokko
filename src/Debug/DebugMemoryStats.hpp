#pragma once

#include "Math/Rectangle.hpp"

class AllocatorManager;
class DebugTextRenderer;

class DebugMemoryStats
{
private:
	AllocatorManager* allocatorManager;
	DebugTextRenderer* textRenderer;

	Rectanglef drawArea;

public:
	DebugMemoryStats(AllocatorManager* allocatorManager, DebugTextRenderer* textRenderer);
	~DebugMemoryStats();

	void SetDrawArea(const Rectanglef& area);

	void UpdateAndDraw();
};
