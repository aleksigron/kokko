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
	int w = glfwGetKey(window, GLFW_KEY_W);
	int a = glfwGetKey(window, GLFW_KEY_A);
	int s = glfwGetKey(window, GLFW_KEY_S);
	int d = glfwGetKey(window, GLFW_KEY_D);

	Transform& cameraTransform = controlledCamera->transform;
	Vec3f dir;

	if (w == GLFW_PRESS)
		dir -= cameraTransform.Forward();
	if (a == GLFW_PRESS)
		dir -= cameraTransform.Right();
	if (s == GLFW_PRESS)
		dir += cameraTransform.Forward();
	if (d == GLFW_PRESS)
		dir += cameraTransform.Right();

	cameraTransform.position += dir * Time::GetDeltaTime();
}
