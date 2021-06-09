#include "Debug/DebugCulling.hpp"

#include "Debug/DebugTextRenderer.hpp"
#include "Debug/DebugVectorRenderer.hpp"

#include "Graphics/Scene.hpp"

#include "Rendering/CameraSystem.hpp"
#include "Rendering/Renderer.hpp"

DebugCulling::DebugCulling(DebugTextRenderer* textRenderer, DebugVectorRenderer* vectorRenderer) :
	renderer(nullptr),
	scene(nullptr),
	cameraSystem(nullptr),
	textRenderer(textRenderer),
	vectorRenderer(vectorRenderer),
	cullingCameraIsLocked(false)
{
}

DebugCulling::~DebugCulling()
{
}

void DebugCulling::Initialize(Renderer* renderer, Scene* scene, CameraSystem* cameraSystem)
{
	this->renderer = renderer;
	this->scene = scene;
	this->cameraSystem = cameraSystem;
}

void DebugCulling::SetLockCullingCamera(bool lockCullingCamera)
{
	cullingCameraIsLocked = lockCullingCamera;
	renderer->SetLockCullingCamera(lockCullingCamera);
}

void DebugCulling::UpdateAndDraw()
{
	if (cullingCameraIsLocked)
	{
		textRenderer->AddText(StringRef("Culling camera is locked"), guideTextPosition);

		const Mat4x4f& transform = renderer->GetCullingCameraTransform();

		Entity cameraEntity = scene->GetActiveCameraEntity();
		CameraId cameraId = cameraSystem->Lookup(cameraEntity);
		ProjectionParameters params = cameraSystem->GetData(cameraId);
		params.perspectiveFar = params.perspectiveFar < 10.0f ? params.perspectiveFar : 10.0f;

		Color white(1.0f, 1.0f, 1.0f);

		vectorRenderer->DrawWireFrustum(transform, params, white);
	}
}
