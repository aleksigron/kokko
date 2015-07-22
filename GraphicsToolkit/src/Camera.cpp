#include "Camera.h"

#include "Matrix.h"

Camera::Camera()
{
}

Camera::~Camera()
{
}

Mat4x4f Camera::GetViewProjectionMatrix()
{
	return this->GetProjectionMatrix() * this->GetViewMatrix();
}

Mat4x4f Camera::GetViewMatrix()
{
	return Matrix::Translate(-this->position) * Matrix::Transpose(this->rotation);
}

Mat4x4f& Camera::GetProjectionMatrix()
{
	if (this->projectionMatrixIsDirty)
	{
		if (this->projectionType == Projection::Perspective)
		{
			this->projectionMatrix = Matrix::Perspective(fieldOfView, aspectRatio, nearClipDistance, farClipDistance);
		}
		else if (this->projectionType == Projection::Orthographic)
		{
			this->projectionMatrix = Matrix::Orthographic(1.0f, 1.0f, nearClipDistance, farClipDistance);
		}
		
		this->projectionMatrixIsDirty = false;
	}
	
	return this->projectionMatrix;
}

void Camera::SetProjectionType(Projection projectionType)
{
	this->projectionMatrixIsDirty = true;
	this->projectionType = projectionType;
}

Camera::Projection Camera::GetProjectionType() const
{
	return this->projectionType;
}

void Camera::SetFieldOfView(float fieldOfView)
{
	this->projectionMatrixIsDirty = true;
	this->fieldOfView = fieldOfView;
}

float Camera::GetFieldOfView() const
{
	return this->fieldOfView;
}

void Camera::SetNearClipDistance(float nearClipDistance)
{
	this->projectionMatrixIsDirty = true;
	this->nearClipDistance = nearClipDistance;
}

float Camera::GetNearClipDistance() const
{
	return this->nearClipDistance;
}

void Camera::SetFarClipDistance(float farClipDistance)
{
	this->projectionMatrixIsDirty = true;
	this->farClipDistance = farClipDistance;
}

float Camera::GetFarClipDistance() const
{
	return this->farClipDistance;
}

void Camera::SetFrameSize(const Vec2i& frameSize)
{
	this->projectionMatrixIsDirty = true;
	this->aspectRatio = frameSize.x / static_cast<float>(frameSize.y);
}