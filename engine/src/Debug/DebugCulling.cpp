#include "Debug/DebugCulling.hpp"

#include "Core/StringView.hpp"

#include "Debug/DebugTextRenderer.hpp"
#include "Debug/DebugVectorRenderer.hpp"

#include "Engine/World.hpp"

#include "Graphics/Scene.hpp"

#include "Rendering/CameraSystem.hpp"
#include "Rendering/Renderer.hpp"

DebugCulling::DebugCulling(DebugTextRenderer* textRenderer, kokko::DebugVectorRenderer* vectorRenderer) :
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

void DebugCulling::UpdateAndDraw(kokko::World* world)
{
	if (cullingCameraIsLocked)
	{
		kokko::Renderer* renderer = world->GetRenderer();
		kokko::CameraSystem* cameraSystem = world->GetCameraSystem();

		textRenderer->AddText(kokko::ConstStringView("Culling camera is locked"), guideTextPosition);

		const Mat4x4f& transform = renderer->GetCullingCameraTransform();

		Entity cameraEntity = cameraSystem->GetActiveCamera();
		kokko::CameraId cameraId = cameraSystem->Lookup(cameraEntity);
		ProjectionParameters params = cameraSystem->GetProjection(cameraId);
		params.perspectiveFar = params.perspectiveFar < 10.0f ? params.perspectiveFar : 10.0f;

		Color white(1.0f, 1.0f, 1.0f);

		vectorRenderer->DrawWireFrustum(transform, params, white);
	}
}
