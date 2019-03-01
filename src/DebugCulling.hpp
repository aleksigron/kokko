#pragma once

#include "Camera.hpp"
#include "CameraController.hpp"

class DebugVectorRenderer;

class DebugCulling
{
private:
	DebugVectorRenderer* vectorRenderer;

	Camera camera;
	CameraController controller;
	bool controllerEnable;

public:
	DebugCulling(DebugVectorRenderer* vectorRenderer);
	~DebugCulling();

	void UpdateAndDraw(Scene* scene, bool controlDebugCamera);

	void EnableOverrideCamera(bool enableDebugCamera);
	void SetControlledCamera(bool enableDebugCamera);

	Camera* GetCamera() { return &camera; }
};
