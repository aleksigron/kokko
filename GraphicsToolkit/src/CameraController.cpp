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

	Transform& cameraTransform = controlledCamera->transform;

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
		Vec2d cursorPos;
		glfwGetCursorPos(window, &cursorPos.x, &cursorPos.y);

		Vec2d move = (cursorPos - prevCursorPos) * 0.005;
		prevCursorPos = cursorPos;

		cameraTransform.rotation = Matrix::Rotate3(cameraTransform.Right(), static_cast<float>(-move.y)) *
		Matrix::Rotate3(cameraTransform.Up(), static_cast<float>(-move.x)) * cameraTransform.rotation;
	}

	Vec3f dir;

	dir -= float(int(kb->GetKey(Key::Q))) * cameraTransform.Up();
	dir -= float(int(kb->GetKey(Key::W))) * cameraTransform.Forward();
	dir += float(int(kb->GetKey(Key::E))) * cameraTransform.Up();
	dir -= float(int(kb->GetKey(Key::A))) * cameraTransform.Right();
	dir += float(int(kb->GetKey(Key::S))) * cameraTransform.Forward();
	dir += float(int(kb->GetKey(Key::D))) * cameraTransform.Right();

	if (dir.SqrMagnitude() > 1.0f)
		dir.Normalize();

	float targetSpeed = cameraMaximumSpeed;

	if (kb->GetKey(Key::LeftShift))
		targetSpeed *= 2.0f;

	cameraVelocity += (dir * targetSpeed - cameraVelocity) * 0.15f;

	cameraTransform.position += cameraVelocity * Time::GetDeltaTime();
}
