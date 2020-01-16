#include "Debug/DebugCulling.hpp"

#include "App.hpp"
#include "Engine.hpp"
#include "EntityManager.hpp"
#include "Window.hpp"
#include "Renderer.hpp"
#include "Scene.hpp"
#include "Math/Frustum.hpp"

#include "Debug/DebugTextRenderer.hpp"
#include "Debug/DebugVectorRenderer.hpp"

DebugCulling::DebugCulling(DebugTextRenderer* textRenderer, DebugVectorRenderer* vectorRenderer):
	textRenderer(textRenderer),
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

void DebugCulling::UpdateAndDraw(Scene* scene)
{
	if (camera.GetEntity().IsNull()) // Initialize culling debug camera
	{
		Engine* engine = Engine::GetInstance();

		// Get main camera transform

		SceneObjectId mainCamScId = scene->Lookup(scene->GetActiveCamera()->GetEntity());
		Mat4x4f mainCamTrans = scene->GetWorldTransform(mainCamScId);

		// Set correct aspect ratio to debug camera

		Vec2f s = engine->GetMainWindow()->GetFrameBufferSize();
		camera.parameters.SetAspectRatio(s.x, s.y);

		// Create entity for debug camera

		Entity cameraEntity = engine->GetEntityManager()->Create();
		camera.SetEntity(cameraEntity);

		// Create scene object for debug camera

		SceneObjectId cameraSceneObject = scene->AddSceneObject(cameraEntity);
		scene->SetLocalTransform(cameraSceneObject, mainCamTrans);
	}

	StringRef text;

	if (controllerEnable)
	{
		text = StringRef("Controlling culling debug camera");
		this->controller.Update();
	}
	else
		text = StringRef("Controlling main camera");

	textRenderer->AddText(text, guideTextPosition);

	Camera* sceneCamera = scene->GetActiveCamera();
	Entity sceneCameraEntity = sceneCamera->GetEntity();
	SceneObjectId sceneCameraSceneObj = scene->Lookup(sceneCameraEntity);
	const Mat4x4f& t = scene->GetWorldTransform(sceneCameraSceneObj);

	Color white(1.0f, 1.0f, 1.0f);

	vectorRenderer->DrawWireFrustum(t, camera.parameters, white);
}
