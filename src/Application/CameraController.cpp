#include "Application/CameraController.hpp"

#include <cstdio>
#include <cstdlib>

#include "Core/String.hpp"

#include "Math/Vec2.hpp"
#include "Math/Mat3x3.hpp"
#include "Math/Mat4x4.hpp"

#include "System/Time.hpp"
#include "System/InputManager.hpp"
#include "System/InputView.hpp"

#include "Scene/World.hpp"

#include "App.hpp"

void CameraController::SetControlledCamera(Entity cameraEntity)
{
	controlledCamera = cameraEntity;
}

void CameraController::VerifySensitityIsLoaded(const ScriptContext& context)
{
	if (cameraAimSensitivity < 0.0f)
	{
		App* app = static_cast<App*>(context.app);
		AppSettings* appSettings = app->GetSettings();

		double sensitivity = 0.0;
		if (appSettings->TryGetDouble("camera_aim_sensitivity", sensitivity))
		{
			this->cameraAimSensitivity = static_cast<float>(sensitivity);
		}
		else
		{
			this->cameraAimSensitivity = 1.0f;

			appSettings->SetDouble("camera_aim_sensitivity", this->cameraAimSensitivity);
			appSettings->SaveToFile();
		}
	}
}

void CameraController::OnUpdate(const ScriptContext& context)
{
	static const int MouseButtonGrab = 0;
	static const int MouseButtonLook = 1;

	this->VerifySensitityIsLoaded(context);

	Scene* world = context.world;

	InputManager* inputManager = context.inputManager;
	InputView* input = inputManager->GetGameInputView();

	// Update mouseLookActive state
	if (mouseLookActive == false && input->GetMouseButtonDown(MouseButtonLook))
	{
		mouseLookActive = true;
		inputManager->SetCursorMode(InputManager::CursorMode::Disabled);
	}
	else if (mouseLookActive == true && input->GetMouseButtonUp(MouseButtonLook))
	{
		mouseLookActive = false;
		inputManager->SetCursorMode(InputManager::CursorMode::Normal);
	}

	// Update mouse look
	if (mouseLookActive == true)
	{
		Vec2f movement = input->GetCursorMovement() * 0.003f * this->cameraAimSensitivity;

		cameraYaw += movement.x;

		if (cameraYaw < -Math::Const::Pi)
			cameraYaw += Math::Const::Tau;
		else if (cameraYaw >= Math::Const::Pi)
			cameraYaw -= Math::Const::Tau;

		cameraPitch += movement.y;

		if (cameraPitch < Math::DegreesToRadians(-89.0f))
			cameraPitch = Math::DegreesToRadians(-89.0f);
		else if (cameraPitch > Math::DegreesToRadians(89.0f))
			cameraPitch = Math::DegreesToRadians(89.0f);
	}

	Mat3x3f rotation = Mat3x3f::RotateAroundAxis(Vec3f(0.0f, 1.0f, 0.0f), -cameraYaw) *
	Mat3x3f::RotateAroundAxis(Vec3f(1.0f, 0.0f, 0.0f), -cameraPitch);

	Vec3f up = rotation.Up();
	Vec3f right = rotation.Right();
	Vec3f forward = rotation.Forward();

	SceneObjectId cameraSceneObject = world->Lookup(controlledCamera);
	Mat4x4f currentTransform = world->GetLocalTransform(cameraSceneObject);
	Vec3f position = (currentTransform * Vec4f(0.0f, 0.0f, 0.0f, 1.0f)).xyz();

	if (mouseLookActive == false)
	{
		if (input->GetMouseButtonDown(MouseButtonGrab))
			mouseGrabActive = true;
		else if (input->GetMouseButtonUp(MouseButtonGrab))
			mouseGrabActive = false;

		if (mouseGrabActive)
		{
			Vec2f movement = input->GetCursorMovement() * 0.015f;
			position += right * -movement.x + up * movement.y;
		}
	}

	Vec3f dir;

	dir -= float(int(input->GetKey(KeyCode::Q))) * up;
	dir += float(int(input->GetKey(KeyCode::W))) * forward;
	dir += float(int(input->GetKey(KeyCode::E))) * up;
	dir -= float(int(input->GetKey(KeyCode::A))) * right;
	dir -= float(int(input->GetKey(KeyCode::S))) * forward;
	dir += float(int(input->GetKey(KeyCode::D))) * right;

	if (dir.SqrMagnitude() > 1.0f)
		dir.Normalize();

	float targetSpeed = 4.0f;

	if (input->GetKey(KeyCode::LeftShift))
		targetSpeed *= 4.0f;

	if (input->GetKey(KeyCode::LeftControl))
		targetSpeed *= 0.125f;

	cameraVelocity += (dir * targetSpeed - cameraVelocity) * Time::GetDeltaTime() * 10.0f;
	position += cameraVelocity * Time::GetDeltaTime();

	Mat4x4f newTransform = Mat4x4f::Translate(position) * Mat4x4f(rotation);
	world->SetLocalTransform(cameraSceneObject, newTransform);
}
