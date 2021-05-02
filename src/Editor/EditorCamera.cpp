#include "Editor/EditorCamera.hpp"

#include "Math/Math.hpp"
#include "Math/Mat4x4.hpp"

#include "System/InputManager.hpp"
#include "System/InputView.hpp"
#include "System/Time.hpp"

EditorCamera::EditorCamera() :
	inputManager(nullptr),
	cameraVelocity(),
	cameraYaw(0.0f),
	cameraPitch(0.0f),
	mouseLookActive(false),
	mouseGrabActive(false),
	cameraAimSensitivity(1.0f)
{
	projection.SetPerspective(Math::DegreesToRadians(60.0f));
	projection.perspectiveNear = 0.1f;
	projection.perspectiveFar = 10000.0f;
	projection.SetAspectRatio(16.0f, 9.0f);
}

void EditorCamera::SetInputManager(InputManager* inputManager)
{
	this->inputManager = inputManager;
}

void EditorCamera::Update()
{
	static const int MouseButtonGrab = 0;
	static const int MouseButtonLook = 1;

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
		// TODO: Use some reasonable multiplier, not a magic number
		Vec2f movement = input->GetCursorMovement() * 0.003f * cameraAimSensitivity;

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

	Mat3x3f orientation = GetOrientation(cameraYaw, cameraPitch);

	Vec3f up = orientation.Up();
	Vec3f right = orientation.Right();
	Vec3f forward = orientation.Forward();

	if (mouseLookActive == false)
	{
		if (input->GetMouseButtonDown(MouseButtonGrab))
			mouseGrabActive = true;
		else if (input->GetMouseButtonUp(MouseButtonGrab))
			mouseGrabActive = false;

		if (mouseGrabActive)
		{
			Vec2f movement = input->GetCursorMovement() * 0.015f;
			cameraPosition += right * -movement.x + up * movement.y;
		}
	}

	Vec3f dir;

	if (mouseGrabActive == false)
	{
		dir -= float(int(input->GetKey(KeyCode::Q))) * up;
		dir += float(int(input->GetKey(KeyCode::W))) * forward;
		dir += float(int(input->GetKey(KeyCode::E))) * up;
		dir -= float(int(input->GetKey(KeyCode::A))) * right;
		dir -= float(int(input->GetKey(KeyCode::S))) * forward;
		dir += float(int(input->GetKey(KeyCode::D))) * right;

		if (dir.SqrMagnitude() > 1.0f)
			dir.Normalize();
	}

	float targetSpeed = 4.0f;

	if (input->GetKey(KeyCode::LeftShift))
		targetSpeed *= 4.0f;

	if (input->GetKey(KeyCode::LeftControl))
		targetSpeed *= 0.125f;

	cameraVelocity += (dir * targetSpeed - cameraVelocity) * Time::GetDeltaTime() * 10.0f;
	cameraPosition += cameraVelocity * Time::GetDeltaTime();
}

Mat4x4fBijection EditorCamera::GetCameraTransform() const
{
	Mat4x4fBijection transform;

	Mat3x3f orientation = GetOrientation(cameraYaw, cameraPitch);
	Mat3x3f inverseOrientation = orientation.GetTransposed();
	Vec3f inverseTranslation = -(inverseOrientation * cameraPosition);

	transform.forward = Mat4x4f::Translate(cameraPosition) * Mat4x4f(orientation);
	transform.inverse = Mat4x4f::Translate(inverseTranslation) * Mat4x4f(inverseOrientation);

	return transform;
}

ProjectionParameters EditorCamera::GetProjectionParameters() const
{
	return projection;
}

Mat3x3f EditorCamera::GetOrientation(float yaw, float pitch)
{
	return Mat3x3f::RotateAroundAxis(Vec3f(0.0f, -1.0f, 0.0f), yaw) *
		Mat3x3f::RotateAroundAxis(Vec3f(-1.0f, 0.0f, 0.0f), pitch);
}