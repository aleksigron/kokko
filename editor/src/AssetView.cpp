#include "AssetView.hpp"

#include "imgui.h"

#include "Core/Core.hpp"

#include "AssetLibrary.hpp"
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
	this->materialManager = materialManager;
}

void AssetView::Update(EditorContext& context)
{
	KOKKO_PROFILE_FUNCTION();

	if (windowIsOpen)
	{
		if (ImGui::Begin(windowTitle, &windowIsOpen))
		{
			World* world = context.world;

			if (context.selectedAsset.HasValue())
			{
				char uidStr[Uid::StringLength + 1];
				context.selectedAsset.GetValue().WriteTo(uidStr);
				uidStr[Uid::StringLength] = '\0';

				ImGui::Text("Selected asset UID: %s", uidStr);
			}
		}

		if (requestFocus)
			ImGui::SetWindowFocus();

		ImGui::End();
	}
}

}
}
