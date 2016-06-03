#include "CameraController.hpp"

#include "App.hpp"
#include "Time.hpp"
#include "PointerInput.hpp"
#include "KeyboardInput.hpp"

void CameraController::SetControlledCamera(Camera* camera)
{
	controlledCamera = camera;
}

void CameraController::Update()
{
	TransformSource& ct = controlledCamera->transform;

	Window* mainWindow = App::GetMainWindow();
	PointerInput* pi = mainWindow->GetPointerInput();
	KeyboardInput* kb = mainWindow->GetKeyboardInput();

	if (kb->GetKeyDown(Key::Space) && mouseGrabActive == false)
	{
		mouseLookEnable = mouseLookEnable == false;
		pi->SetCursorMode(mouseLookEnable ?
						  PointerInput::CursorMode::Disabled :
						  PointerInput::CursorMode::Normal);

	}

	if (mouseLookEnable)
	{
		Vec2f movement = pi->GetCursorMovement() * 0.003f;

		cameraYaw += movement.x;

		if (cameraYaw < -180.0f)
			cameraYaw += 360.0f;
		else if (cameraYaw >= 180.0f)
			cameraYaw -= 360.0f;

		cameraPitch += movement.y;

		if (cameraPitch < -80.0f)
			cameraPitch = -80.0f;
		else if (cameraPitch > 80.0f)
			cameraPitch = 80.0f;

		ct.rotation = Mat3x3f::RotateAroundAxis(Vec3f(0.0f, 1.0f, 0.0f), -cameraYaw) *
		Mat3x3f::RotateAroundAxis(Vec3f(1.0f, 0.0f, 0.0f), -cameraPitch);
	}
	else
	{
		if (pi->GetMouseButtonDown(0))
			mouseGrabActive = true;
		else if (pi->GetMouseButtonUp(0))
			mouseGrabActive = false;

		if (mouseGrabActive)
		{
			Vec2f movement = pi->GetCursorMovement() * 0.015f;
			ct.position += ct.Right() * -movement.x + ct.Up() * movement.y;
		}
	}

	Vec3f dir;

	dir -= float(int(kb->GetKey(Key::Q))) * ct.Up();
	dir += float(int(kb->GetKey(Key::W))) * ct.Forward();
	dir += float(int(kb->GetKey(Key::E))) * ct.Up();
	dir -= float(int(kb->GetKey(Key::A))) * ct.Right();
	dir -= float(int(kb->GetKey(Key::S))) * ct.Forward();
	dir += float(int(kb->GetKey(Key::D))) * ct.Right();

	if (dir.SqrMagnitude() > 1.0f)
		dir.Normalize();

	float targetSpeed = cameraMaximumSpeed;

	if (kb->GetKey(Key::LeftShift))
		targetSpeed *= 2.0f;

	cameraVelocity += (dir * targetSpeed - cameraVelocity) * 0.15f;

	ct.position += cameraVelocity * Time::GetDeltaTime();
}
