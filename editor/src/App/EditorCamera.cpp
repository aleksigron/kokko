#include "EditorCamera.hpp"

#include "imgui.h"

#include "Math/Math.hpp"
#include "Math/Mat3x3.hpp"
#include "Math/Mat4x4.hpp"

#include "Platform/Window.hpp"

#include "Rendering/CameraParameters.hpp"

#include "System/KeyCode.hpp"

namespace kokko
{
namespace editor
{

EditorCamera::EditorCamera() :
	window(nullptr),
	cameraVelocity(),
	cameraYaw(0.0f),
	cameraPitch(0.0f),
	mouseLookActive(false),
	cameraAimSensitivity(1.0f)
{
	projection.SetPerspective(Math::DegreesToRadians(60.0f));
	projection.perspectiveNear = 0.1f;
	projection.perspectiveFar = 10000.0f;
	projection.SetAspectRatio(16.0f, 9.0f);
}

void EditorCamera::SetWindow(Window* window)
{
	this->window = window;
}

void EditorCamera::LookAt(const Vec3f& position, const Vec3f& lookAtTarget)
{
	cameraPosition = position;

	Vec3f diff = lookAtTarget - position;
	float xzDistance = diff.xz().Magnitude();

	cameraYaw = std::atan2(diff.x, -diff.z);
	cameraPitch = std::atan2(-diff.y, xzDistance);
}

void EditorCamera::SetAspectRatio(float width, float height)
{
	projection.SetAspectRatio(width, height);
}

static float GetKeyValue0to1(ImGuiIO& io, KeyCode key)
{
	return io.KeysDown[static_cast<size_t>(key)] ? 1.0f : 0.0f;
}

void EditorCamera::Update(bool windowIsActive)
{
	static const ImGuiMouseButton MouseButtonLook = ImGuiMouseButton_Right;

	float targetSpeed = 4.0f;
	Vec3f moveDir;

	ImGuiIO& io = ImGui::GetIO();

	if (windowIsActive && io.WantTextInput == false)
	{
		// Update mouseLookActive state
		if (mouseLookActive == false && ImGui::IsMouseDown(MouseButtonLook))
		{
			mouseLookActive = true;
			window->SetCursorMode(Window::CursorMode::Disabled);
		}
		else if (mouseLookActive == true && ImGui::IsMouseDown(MouseButtonLook) == false)
		{
			mouseLookActive = false;
			window->SetCursorMode(Window::CursorMode::Normal);
		}

		Vec2f mouseDelta(io.MouseDelta.x, io.MouseDelta.y);

		// Update mouse look
		if (mouseLookActive == true)
		{
			// TODO: Use some reasonable multiplier, not a magic number

			Vec2f movement = mouseDelta * 0.003f * cameraAimSensitivity;

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

		moveDir -= GetKeyValue0to1(io, KeyCode::Q) * up;
		moveDir += GetKeyValue0to1(io, KeyCode::W) * forward;
		moveDir += GetKeyValue0to1(io, KeyCode::E) * up;
		moveDir -= GetKeyValue0to1(io, KeyCode::A) * right;
		moveDir -= GetKeyValue0to1(io, KeyCode::S) * forward;
		moveDir += GetKeyValue0to1(io, KeyCode::D) * right;

		if (moveDir.SqrMagnitude() > 1.0f)
			moveDir.Normalize();

		if (io.KeyMods & ImGuiModFlags_Shift)
			targetSpeed *= 4.0f;

		if (io.KeyMods & ImGuiModFlags_Ctrl)
			targetSpeed *= 0.125f;
	}

	float dt = io.DeltaTime;

	cameraVelocity += (moveDir * targetSpeed - cameraVelocity) * dt * 10.0f;
	cameraPosition += cameraVelocity * dt;
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

CameraParameters EditorCamera::GetCameraParameters() const
{
	Mat4x4fBijection cameraTransform = GetCameraTransform();
	ProjectionParameters cameraProjection = GetProjectionParameters();

	return CameraParameters{ cameraTransform, cameraProjection };
}

Mat3x3f EditorCamera::GetOrientation(float yaw, float pitch)
{
	return Mat3x3f::RotateAroundAxis(Vec3f(0.0f, -1.0f, 0.0f), yaw) *
		Mat3x3f::RotateAroundAxis(Vec3f(-1.0f, 0.0f, 0.0f), pitch);
}

}
}
