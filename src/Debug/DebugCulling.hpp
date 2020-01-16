#pragma once

#include "Math/Vec2.hpp"
#include "Rendering/Camera.hpp"
#include "CameraController.hpp"

class DebugTextRenderer;
class DebugVectorRenderer;

class DebugCulling
{
private:
	DebugTextRenderer* textRenderer;
	DebugVectorRenderer* vectorRenderer;

	Camera camera;
	CameraController controller;
	bool controllerEnable;

	Vec2f guideTextPosition;

public:
	DebugCulling(DebugTextRenderer* textRenderer, DebugVectorRenderer* vectorRenderer);
	~DebugCulling();

	void UpdateAndDraw(Scene* scene);

	void EnableOverrideCamera(bool enableDebugCamera);
	void SetControlledCamera(bool enableDebugCamera);
	void SetGuideTextPosition(const Vec2f& pos) { guideTextPosition = pos; }

	Camera* GetCamera() { return &camera; }
};
