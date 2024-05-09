#pragma once

#include "Math/Rectangle.hpp"

class AllocatorManager;

namespace kokko
{
class DebugTextRenderer;
}

class DebugMemoryStats
{
private:
	AllocatorManager* allocatorManager;
	kokko::DebugTextRenderer* textRenderer;

	Rectanglef drawArea;

public:
	DebugMemoryStats(AllocatorManager* allocatorManager, kokko::DebugTextRenderer* textRenderer);
	~DebugMemoryStats();

	void SetDrawArea(const Rectanglef& area);

	void UpdateAndDraw();
};
