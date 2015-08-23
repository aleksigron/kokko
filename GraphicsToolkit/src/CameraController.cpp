#include "CameraController.h"

#include "App.h"
#include "Time.h"

#define GLFW_INCLUDE_NONE
#include "glfw/glfw3.h"

void CameraController::SetControlledCamera(Camera* camera)
{
	controlledCamera = camera;
}

void CameraController::Update()
{
	GLFWwindow* window = App::GetMainWindow()->GetWindowHandle();

	Transform& ct = controlledCamera->transform;

	KeyboardInput* kb = &(App::GetInput()->keyboard);

	if (kb->GetKeyDown(Key::Space))
	{
		if (mouseControlEnable)
		{
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
			mouseControlEnable = false;
		}
		else
		{
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
			mouseControlEnable = true;
		}
	}

	if (mouseControlEnable)
	{
		PointerInput* pi = &(App::GetInput()->pointer);
		Vec2f movement = pi->GetCursorMovement() * 0.004f;

		ct.rotation = Mat3x3f::RotateAroundAxis(ct.Right(), -movement.y) *
		Mat3x3f::RotateAroundAxis(ct.Up(), -movement.x) * ct.rotation;
	}

	Vec3f dir;

	dir -= float(int(kb->GetKey(Key::Q))) * ct.Up();
	dir -= float(int(kb->GetKey(Key::W))) * ct.Forward();
	dir += float(int(kb->GetKey(Key::E))) * ct.Up();
	dir -= float(int(kb->GetKey(Key::A))) * ct.Right();
	dir += float(int(kb->GetKey(Key::S))) * ct.Forward();
	dir += float(int(kb->GetKey(Key::D))) * ct.Right();

	if (dir.SqrMagnitude() > 1.0f)
		dir.Normalize();

	float targetSpeed = cameraMaximumSpeed;

	if (kb->GetKey(Key::LeftShift))
		targetSpeed *= 2.0f;

	cameraVelocity += (dir * targetSpeed - cameraVelocity) * 0.15f;

	ct.position += cameraVelocity * Time::GetDeltaTime();
}
