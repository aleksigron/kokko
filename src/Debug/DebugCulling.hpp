#pragma once

#include "Math/Vec2.hpp"

class Scene;
class Renderer;
class CameraSystem;
class DebugTextRenderer;
class DebugVectorRenderer;

class DebugCulling
{
private:
	Renderer* renderer;
	CameraSystem* cameraSystem;
	DebugTextRenderer* textRenderer;
	DebugVectorRenderer* vectorRenderer;

	bool cullingCameraIsLocked;

	Vec2f guideTextPosition;

public:
	DebugCulling(DebugTextRenderer* textRenderer, DebugVectorRenderer* vectorRenderer);
	~DebugCulling();

	void Initialize(Renderer* renderer, CameraSystem* cameraSystem);

	void UpdateAndDraw(Scene* scene);

	void SetLockCullingCamera(bool lockCullingCamera);
	void SetGuideTextPosition(const Vec2f& pos) { guideTextPosition = pos; }
};
