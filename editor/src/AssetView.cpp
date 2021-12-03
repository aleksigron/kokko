#include "AssetView.hpp"

#include "imgui.h"

#include "Core/Core.hpp"

#include "EditorContext.hpp"

namespace kokko
{
namespace editor
{

AssetView::AssetView() :
	EditorWindow("Asset"),
	materialManager(nullptr)
{
}

void AssetView::Initialize(MaterialManager* materialManager)
{
}

void AssetView::Update(EditorContext& context)
{
	KOKKO_PROFILE_FUNCTION();

	if (windowIsOpen)
	{
		if (ImGui::Begin(windowTitle, &windowIsOpen))
		{
			World* world = context.world;

			if (context.selectedAsset)
			{
				ImGui::Text("Asset is selected");
			}
		}

		if (requestFocus)
			ImGui::SetWindowFocus();

		ImGui::End();
	}
}

}
}
