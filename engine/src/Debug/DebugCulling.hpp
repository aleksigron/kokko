#pragma once

#include "Math/Vec2.hpp"

class DebugTextRenderer;

namespace kokko
{
class DebugVectorRenderer;
class World;
}

class DebugCulling
{
private:
	DebugTextRenderer* textRenderer;
	kokko::DebugVectorRenderer* vectorRenderer;

	bool cullingCameraIsLocked;

	Vec2f guideTextPosition;

public:
	DebugCulling(DebugTextRenderer* textRenderer, kokko::DebugVectorRenderer* vectorRenderer);
	~DebugCulling();

	void UpdateAndDraw(kokko::World* world);

	void SetLockCullingCamera(bool lockCullingCamera);
	void SetGuideTextPosition(const Vec2f& pos) { guideTextPosition = pos; }
};
