#include "Views/TerrainDebugView.hpp"

#include "imgui.h"

#include "Core/Core.hpp"

#include "Debug/Debug.hpp"

#include "Engine/EngineSettings.hpp"

#include "System/Time.hpp"

#include "App/EditorContext.hpp"

namespace kokko
{
namespace editor
{

TerrainDebugView::TerrainDebugView() :
	EditorWindow("Terrain Debug", EditorWindowGroup::Debug)
{
}

void TerrainDebugView::Initialize(Debug* debug)
{
}

void TerrainDebugView::Update(EditorContext& context)
{
	KOKKO_PROFILE_FUNCTION();

	if (windowIsOpen)
	{
		ImGui::SetNextWindowSize(ImVec2(400, 400), ImGuiCond_FirstUseEver);
		if (ImGui::Begin(windowTitle, &windowIsOpen))
		{
			kokko::RenderDebugSettings& features = context.engineSettings->renderDebug;

			bool drawTerrain = features.IsFeatureEnabled(kokko::RenderDebugFeatureFlag::DrawTerrainTiles);
			if (ImGui::Checkbox("Terrain tile debug", &drawTerrain))
				features.SetFeatureEnabled(kokko::RenderDebugFeatureFlag::DrawTerrainTiles, drawTerrain);

			bool terrainShadows = features.IsFeatureEnabled(kokko::RenderDebugFeatureFlag::ExperimentalTerrainShadows);
			if (ImGui::Checkbox("Experimental terrain shadows", &terrainShadows))
				features.SetFeatureEnabled(kokko::RenderDebugFeatureFlag::ExperimentalTerrainShadows, terrainShadows);
		}

		if (requestFocus)
			ImGui::SetWindowFocus();

		ImGui::End();
	}
}

} // namespace editor
} // namespace kokko
