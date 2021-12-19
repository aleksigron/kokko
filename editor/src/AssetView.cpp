#include "AssetView.hpp"

#include "imgui.h"

#include "Core/Core.hpp"

#include "Rendering/Uniform.hpp"
#include "Rendering/UniformData.hpp"

#include "Resources/MaterialManager.hpp"
#include "Resources/TextureManager.hpp"

#include "AssetLibrary.hpp"
#include "EditorContext.hpp"

namespace kokko
{
namespace editor
{

AssetView::AssetView(Allocator* allocator) :
	EditorWindow("Asset"),
	materialManager(nullptr),
	textStore(allocator)
{
}

void AssetView::Initialize(MaterialManager* materialManager, TextureManager* textureManager)
{
	this->materialManager = materialManager;
	this->textureManager = textureManager;
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

				auto asset = context.assetLibrary->FindAssetByUid(uid);
				if (asset == nullptr)
				{
					char uidStr[Uid::StringLength + 1];
					uid.WriteTo(uidStr);
					uidStr[Uid::StringLength] = '\0';

					KK_LOG_ERROR("Asset with UID {} not found in AssetLibrary", uidStr);
				}
				else
				{
					if (asset->type == AssetType::Material)
					{
						DrawMaterial(asset);
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

void AssetView::DrawMaterial(const AssetInfo* asset)
{
	MaterialId materialId = materialManager->FindMaterialByUid(asset->uid);
	if (materialId == MaterialId::Null)
	{
		ImGui::Text("Material not found in MaterialManager");
		return;
	}

	ShaderId shaderId = materialManager->GetMaterialShader(materialId);

	ImGui::Text("Shader ID: %u", shaderId.i);

	UniformData& uniforms = materialManager->GetMaterialUniforms(materialId);

	if (ImGui::CollapsingHeader("Properties", ImGuiTreeNodeFlags_DefaultOpen))
	{
		for (auto& uniform : uniforms.GetBufferUniforms())
		{
			DrawMaterialProperty(uniforms, uniform);
		}
	}
	
	if (ImGui::CollapsingHeader("Textures", ImGuiTreeNodeFlags_DefaultOpen))
	{
		for (auto& texture : uniforms.GetTextureUniforms())
		{
			textStore.Assign(texture.name);
			ImGui::Text("%s", textStore.GetCStr());

			if (texture.textureId == TextureId::Null)
			{
				ImGui::Text("No texture");
			}
			else
			{
				ImVec2 uv0(0.0f, 1.0f);
				ImVec2 uv1(1.0f, 0.0f);

				float side = ImGui::GetFontSize() * 6.0f;
				ImVec2 size(side, side);

				void* texId = reinterpret_cast<void*>(static_cast<size_t>(texture.textureObject));

				ImGui::Image(texId, size, uv0, uv1);
			}

			ImGui::Spacing();
		}
	}
}

bool AssetView::DrawMaterialProperty(UniformData& uniforms, const BufferUniform& prop)
{
	bool edited = false;

	textStore.Assign(prop.name);

	const char* unsupportedPropertyType = "unknown";

	switch (prop.type)
	{
	case UniformDataType::Vec4:
	{
		auto value = uniforms.GetValue<Vec4f>(prop);
		edited = ImGui::InputFloat4(textStore.GetCStr(), value.ValuePointer());
		break;
	}

	case UniformDataType::Vec3:
	{
		auto value = uniforms.GetValue<Vec3f>(prop);
		edited = ImGui::InputFloat3(textStore.GetCStr(), value.ValuePointer());
		break;
	}

	case UniformDataType::Vec2:
	{
		auto value = uniforms.GetValue<Vec2f>(prop);
		edited = ImGui::InputFloat2(textStore.GetCStr(), value.ValuePointer());
		break;
	}

	case UniformDataType::Float:
	{
		auto value = uniforms.GetValue<float>(prop);
		edited = ImGui::InputFloat(textStore.GetCStr(), &value);
		break;
	}

	case UniformDataType::Int:
	{
		auto value = uniforms.GetValue<int>(prop);
		edited = ImGui::InputInt(textStore.GetCStr(), &value);
		break;
	}

	default:
		ImGui::Text("Unsupported property type: %s", unsupportedPropertyType);
		break;
	}

	ImGui::Spacing();

	return edited;
}

}
}
