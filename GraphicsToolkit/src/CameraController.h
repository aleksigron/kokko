#pragma once

#include "Vec3.h"

struct Camera;

class CameraController
{
private:
	Camera* controlledCamera = nullptr;
	Vec3f position;

public:
	void SetControlledCamera(Camera* camera);
	void Update();
};
