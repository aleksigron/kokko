#include "DebugCulling.hpp"

#include "App.hpp"
#include "Engine.hpp"
#include "Window.hpp"
#include "Renderer.hpp"
#include "Scene.hpp"
#include "ViewFrustum.hpp"
#include "DebugVectorRenderer.hpp"

DebugCulling::DebugCulling(DebugVectorRenderer* vectorRenderer) :
	vectorRenderer(vectorRenderer),
	controllerEnable(false)
{
	this->controller.SetControlledCamera(&camera);
}

DebugCulling::~DebugCulling()
{
}

void DebugCulling::EnableOverrideCamera(bool enableDebugCamera)
{
	Camera* overrideCamera = enableDebugCamera ? &camera : nullptr;
	Engine::GetInstance()->GetRenderer()->SetRenderCameraOverride(overrideCamera);
}

void DebugCulling::SetControlledCamera(bool enableDebugCamera)
{
	this->controllerEnable = enableDebugCamera;
	App::GetInstance()->SetCameraControllerEnable(!enableDebugCamera);
}

void DebugCulling::UpdateAndDraw(Scene* scene, bool controlDebugCamera)
{
	if (controllerEnable)
		this->controller.Update();

	if (this->camera.GetSceneObjectId() == 0)
	{
		Vec2f s = Engine::GetInstance()->GetMainWindow()->GetFrameBufferSize();
		this->camera.SetAspectRatio(s.x, s.y);

		this->camera.InitializeSceneObject(scene);
		unsigned int cameraSceneObj = this->camera.GetSceneObjectId();
		scene->SetLocalTransform(cameraSceneObj, Mat4x4f());
	}

	Camera* camera = scene->GetActiveCamera();
	unsigned int cameraSceneObject = camera->GetSceneObjectId();

	const Mat4x4f& t = scene->GetWorldTransform(cameraSceneObject);

	ViewFrustum frustum;
	frustum.UpdateFrustum(*camera, t);

	Color white(1.0f, 1.0f, 1.0f);

	vectorRenderer->DrawLine(frustum.points[0], frustum.points[1], white);
	vectorRenderer->DrawLine(frustum.points[0], frustum.points[2], white);
	vectorRenderer->DrawLine(frustum.points[1], frustum.points[3], white);
	vectorRenderer->DrawLine(frustum.points[2], frustum.points[3], white);

	vectorRenderer->DrawLine(frustum.points[0], frustum.points[4], white);
	vectorRenderer->DrawLine(frustum.points[1], frustum.points[5], white);
	vectorRenderer->DrawLine(frustum.points[2], frustum.points[6], white);
	vectorRenderer->DrawLine(frustum.points[3], frustum.points[7], white);

	vectorRenderer->DrawLine(frustum.points[4], frustum.points[5], white);
	vectorRenderer->DrawLine(frustum.points[4], frustum.points[6], white);
	vectorRenderer->DrawLine(frustum.points[5], frustum.points[7], white);
	vectorRenderer->DrawLine(frustum.points[6], frustum.points[7], white);
}
