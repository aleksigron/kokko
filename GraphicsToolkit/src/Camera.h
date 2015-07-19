#pragma once

#include "Transform.h"
#include "Mat4x4.h"
#include "Vec2.h"
#include "Math.h"

class Camera : public Transform
{
public:
	enum class Projection
	{
		Perspective,
		Orthographic
	};
	
private:
	Mat4x4f projectionMatrix;
	
	float fieldOfView = Mathf::DegreesToRadians(45.0f);
	float nearClipDistance = 1.0f;
	float farClipDistance = 100.0f;
	float aspectRatio = 1.0f;
	
	Projection projectionType = Projection::Perspective;
	
	bool projectionMatrixIsDirty = true;
	
public:
	Camera();
	~Camera();
	
	void SetProjectionType(Projection projectionType);
	Projection GetProjectionType() const;
	
	void SetFieldOfView(float fieldOfView);
	float GetFieldOfView() const;
	
	void SetNearClipDistance(float nearClipDistance);
	float GetNearClipDistance() const;
	
	void SetFarClipDistance(float farClipDistance);
	float GetFarClipDistance() const;
	
	void SetFrameSize(const Vec2i& frameSize);
	
	Mat4x4f GetViewProjectionMatrix();
	
	Mat4x4f GetViewMatrix();
	Mat4x4f& GetProjectionMatrix();
};