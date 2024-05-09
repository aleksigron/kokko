#pragma once

#include "Math/Vec2.hpp"

namespace kokko
{
class DebugTextRenderer;
class DebugVectorRenderer;
class World;

class DebugCulling
{
private:
	kokko::DebugTextRenderer* textRenderer;
	kokko::DebugVectorRenderer* vectorRenderer;

	bool cullingCameraIsLocked;

	Vec2f guideTextPosition;

public:
	DebugCulling(kokko::DebugTextRenderer* textRenderer, kokko::DebugVectorRenderer* vectorRenderer);
	~DebugCulling();

	void UpdateAndDraw(kokko::World* world);

	void SetLockCullingCamera(bool lockCullingCamera);
	void SetGuideTextPosition(const Vec2f& pos) { guideTextPosition = pos; }
};

}
