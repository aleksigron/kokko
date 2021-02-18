#include "Debug/DebugCulling.hpp"

#include "Application/App.hpp"
#include "Engine/Engine.hpp"
#include "Entity/EntityManager.hpp"
#include "System/Window.hpp"
#include "Rendering/Renderer.hpp"
#include "Scene/Scene.hpp"
#include "Math/Frustum.hpp"

#include "Debug/DebugTextRenderer.hpp"
#include "Debug/DebugVectorRenderer.hpp"

DebugCulling::DebugCulling(DebugTextRenderer* textRenderer, DebugVectorRenderer* vectorRenderer):
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
	Engine::GetInstance()->GetRenderer()->SetLockCullingCamera(lockCullingCamera);
}

void DebugCulling::UpdateAndDraw(Scene* scene)
{
	if (cullingCameraIsLocked)
	{
		textRenderer->AddText(StringRef("Culling camera is locked"), guideTextPosition);

		const Mat4x4f& transform = Engine::GetInstance()->GetRenderer()->GetCullingCameraTransform();
		Color white(1.0f, 1.0f, 1.0f);

		vectorRenderer->DrawWireFrustum(transform, scene->GetActiveCamera()->parameters, white);
	}
}
