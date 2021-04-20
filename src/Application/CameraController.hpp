#pragma once

#include "Math/Vec3.hpp"

#include "Scripting/NativeScriptComponent.hpp"

#include "Rendering/CameraId.hpp"

class CameraController : public NativeScriptComponent
{
private:
	Entity controlledCamera = Entity::Null;

	float cameraYaw = 0.0f;
	float cameraPitch = 0.0f;
	
	Vec3f cameraVelocity;

	bool mouseLookActive = false;
	bool mouseGrabActive = false;

	float cameraAimSensitivity = -1.0f;

	void VerifySensitityIsLoaded(const ScriptContext& context);

public:
	void SetControlledCamera(Entity cameraEntity);
	
	virtual void OnUpdate(const ScriptContext& context) override;
};
