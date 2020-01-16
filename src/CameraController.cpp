#include "CameraController.hpp"

#include <cstdio>
#include <cstdlib>

#include "String.hpp"

#include "Math/Mat3x3.hpp"
#include "Math/Mat4x4.hpp"

#include "Engine.hpp"
#include "Time.hpp"
#include "Window.hpp"
#include "InputManager.hpp"
#include "PointerInput.hpp"
#include "KeyboardInputView.hpp"

#include "SceneManager.hpp"
#include "Scene.hpp"

#include "App.hpp"
#include "Rendering/Camera.hpp"

CameraController::CameraController()
{
}

CameraController::~CameraController()
{
}

void CameraController::SetControlledCamera(Camera* camera)
{
	controlledCamera = camera;
}

void CameraController::VerifySensitityIsLoaded()
{
	if (cameraAimSensitivity < 0.0f)
	{
		AppSettings* settings = App::GetInstance()->GetSettings();

		double sensitivity = 0.0;
		if (settings->TryGetDouble("camera_aim_sensitivity", sensitivity))
		{
			this->cameraAimSensitivity = static_cast<float>(sensitivity);
		}
		else
		{
			this->cameraAimSensitivity = 1.0f;

			settings->SetDouble("camera_aim_sensitivity", this->cameraAimSensitivity);
			settings->SaveToFile();
		}
	}
}

void CameraController::Update()
{
	this->VerifySensitityIsLoaded();

	SceneManager* sm = Engine::GetInstance()->GetSceneManager();
	Scene* scene = sm->GetScene(sm->GetPrimarySceneId());

	InputManager* inputManager = Engine::GetInstance()->GetMainWindow()->GetInputManager();
	PointerInput* pi = inputManager->GetPointerInput();
	KeyboardInputView* kb = inputManager->GetKeyboardInputView();

	if (kb->GetKeyDown(Key::Space) && mouseGrabActive == false)
	{
		mouseLookEnable = !mouseLookEnable;

		pi->SetCursorMode(mouseLookEnable ?
						  PointerInput::CursorMode::Disabled :
						  PointerInput::CursorMode::Normal);
	}

	if (mouseLookEnable == true)
	{
		Vec2f movement = pi->GetCursorMovement() * 0.003f * this->cameraAimSensitivity;

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
	}

	Mat3x3f rotation = Mat3x3f::RotateAroundAxis(Vec3f(0.0f, 1.0f, 0.0f), -cameraYaw) *
	Mat3x3f::RotateAroundAxis(Vec3f(1.0f, 0.0f, 0.0f), -cameraPitch);

	Vec3f up = rotation.Up();
	Vec3f right = rotation.Right();
	Vec3f forward = rotation.Forward();

	Entity cameraEntity = controlledCamera->GetEntity();
	SceneObjectId cameraSceneObject = scene->Lookup(cameraEntity);
	Mat4x4f currentTransform = scene->GetLocalTransform(cameraSceneObject);
	Vec3f position = (currentTransform * Vec4f(0.0f, 0.0f, 0.0f, 1.0f)).xyz();

	if (mouseLookEnable == false)
	{
		if (pi->GetMouseButtonDown(0))
			mouseGrabActive = true;
		else if (pi->GetMouseButtonUp(0))
			mouseGrabActive = false;

		if (mouseGrabActive)
		{
			Vec2f movement = pi->GetCursorMovement() * 0.015f;
			position += right * -movement.x + up * movement.y;
		}
	}

	Vec3f dir;

	dir -= float(int(kb->GetKey(Key::Q))) * up;
	dir += float(int(kb->GetKey(Key::W))) * forward;
	dir += float(int(kb->GetKey(Key::E))) * up;
	dir -= float(int(kb->GetKey(Key::A))) * right;
	dir -= float(int(kb->GetKey(Key::S))) * forward;
	dir += float(int(kb->GetKey(Key::D))) * right;

	if (dir.SqrMagnitude() > 1.0f)
		dir.Normalize();

	float targetSpeed = cameraMaximumSpeed;

	if (kb->GetKey(Key::LeftShift))
		targetSpeed *= 2.0f;

	cameraVelocity += (dir * targetSpeed - cameraVelocity) * 0.15f;
	position += cameraVelocity * Time::GetDeltaTime();

	Mat4x4f newTransform = Mat4x4f::Translate(position) * Mat4x4f(rotation);
	scene->SetLocalTransform(cameraSceneObject, newTransform);
}
