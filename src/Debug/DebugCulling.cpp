#include "Debug/DebugCulling.hpp"

#include "Debug/DebugTextRenderer.hpp"
#include "Debug/DebugVectorRenderer.hpp"

#include "Engine/World.hpp"

#include "Graphics/Scene.hpp"

#include "Rendering/CameraSystem.hpp"
#include "Rendering/Renderer.hpp"

DebugCulling::DebugCulling(DebugTextRenderer* textRenderer, DebugVectorRenderer* vectorRenderer) :
	textRenderer(textRenderer),
	vectorRenderer(vectorRenderer),
	cullingCameraIsLocked(false)
{
}

DebugCulling::~DebugCulling()
{
}

void DebugCulling::SetLockCullingCamera(bool lockCullingCamera)
{
	cullingCameraIsLocked = lockCullingCamera;
}

void DebugCulling::UpdateAndDraw(World* world)
{
	if (cullingCameraIsLocked)
	{
		Renderer* renderer = world->GetRenderer();
		Scene* scene = world->GetScene();
		CameraSystem* cameraSystem = world->GetCameraSystem();

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
