#pragma once

#include "Math/Vec3.hpp"

#include "Scripting/NativeScriptComponent.hpp"

class Camera;

class CameraController : public NativeScriptComponent
{
private:
	Camera* controlledCamera = nullptr;

	float cameraYaw = 0.0f;
	float cameraPitch = 0.0f;
	
	Vec3f cameraVelocity;

	bool mouseLookActive = false;
	bool mouseGrabActive = false;

	float cameraAimSensitivity = -1.0f;

	void VerifySensitityIsLoaded(const ScriptContext& context);

public:
	void SetControlledCamera(Camera* camera);
	
	virtual void OnUpdate(const ScriptContext& context) override;
};
