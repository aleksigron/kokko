#include "Camera.hpp"

#include "Engine.hpp"
#include "SceneManager.hpp"
#include "Scene.hpp"

Camera::Camera() :
	sceneId(0),
	sceneObjectId(0)
{
}

Camera::~Camera()
{
}

void Camera::InitializeSceneObject(unsigned int sceneId)
{
	this->sceneId = sceneId;

	if (sceneObjectId == 0)
	{
		Scene* scene = this->GetContainingScene();
		this->sceneObjectId = scene->AddSceneObject();
	}
}

Scene* Camera::GetContainingScene() const
{
	SceneManager* sm = Engine::GetInstance()->GetSceneManager();
	return sm->GetScene(sceneId);
}

Mat4x4f Camera::GetViewMatrix() const
{
	Scene* scene = this->GetContainingScene();
	Mat4x4f m = scene->GetWorldTransform(sceneObjectId);
	Mat3x3f inverseRotation = m.Get3x3().GetTransposed();
	Vec3f translation = -(inverseRotation * Vec3f(m[12], m[13], m[14]));

	Mat4x4f v(inverseRotation);
	v[12] = translation.x;
	v[13] = translation.y;
	v[14] = translation.z;
	 
	return v;
}

Mat4x4f Camera::GetProjectionMatrix() const
{
	Mat4x4f result;

	const float farMinusNear = farClipDistance - nearClipDistance;
	const float farPlusNear = farClipDistance + nearClipDistance;

	if (projectionType == Projection::Perspective)
	{
		const float tanHalfFovy = std::tan(perspectiveFieldOfView * 0.5f);
		const float farTimesNear = farClipDistance * nearClipDistance;

		result[0] = 1.0f / (aspectRatio * tanHalfFovy);
		result[5] = 1.0f / (tanHalfFovy);
		result[10] = - (farPlusNear) / (farMinusNear);
		result[11] = - 1.0f;
		result[14] = - (2.0f * farTimesNear) / (farMinusNear);
		result[15] = 0.0f;
	}
	else if (projectionType == Projection::Orthographic)
	{
		const float halfHeight = orthogonalHeight * 0.5f;
		const float halfWidth = halfHeight / aspectRatio;

		result[0] = 1.0f / halfWidth;
		result[5] = 1.0f / halfHeight;
		result[10] = -2.0f / (farMinusNear);
		result[14] = - (farPlusNear) / (farMinusNear);
	}

	return result;
}