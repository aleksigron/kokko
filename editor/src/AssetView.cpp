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
				Uid uid = context.selectedAsset.GetValue();

				char uidStr[Uid::StringLength + 1];
				uid.WriteTo(uidStr);
				uidStr[Uid::StringLength] = '\0';

				ImGui::Text("Selected asset UID: %s", uidStr);

				auto asset = context.assetLibrary->FindAssetByUid(uid);
				if (asset == nullptr)
				{
					KK_LOG_ERROR("Asset with UID {} not found in AssetLibrary", uidStr);
				}
				else
				{
					if (asset->type == AssetType::Material)
					{
						ImGui::Text("Selected asset is a material");
					}
					else
					{
						ImGui::Text("Selected asset is an unknown asset type");
					}
				}
			}
		}

		if (requestFocus)
			ImGui::SetWindowFocus();

		ImGui::End();
	}
}

}
}
