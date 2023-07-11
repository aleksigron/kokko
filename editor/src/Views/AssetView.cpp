#include "AssetView.hpp"

#include "imgui.h"

#include "Core/Core.hpp"

#include "Rendering/Uniform.hpp"
#include "Rendering/UniformData.hpp"

#include "Resources/AssetLibrary.hpp"
#include "Resources/MaterialManager.hpp"
#include "Resources/MaterialSerializer.hpp"
#include "Resources/ShaderManager.hpp"
#include "Resources/TextureManager.hpp"

#include "App/EditorConstants.hpp"
#include "App/EditorContext.hpp"

namespace kokko
{
namespace editor
{

AssetView::AssetView(Allocator* allocator) :
	EditorWindow("Asset"),
	allocator(allocator),
	materialManager(nullptr),
	shaderManager(nullptr),
	textureManager(nullptr)
{
}

void AssetView::Initialize(
	MaterialManager* materialManager,
	ShaderManager* shaderManager,
	TextureManager* textureManager)
{
	this->materialManager = materialManager;
	this->shaderManager = shaderManager;
	this->textureManager = textureManager;
}

void AssetView::Update(EditorContext& context)
{
	KOKKO_PROFILE_FUNCTION();

	if (windowIsOpen)
	{
		ImGui::SetNextWindowSize(ImVec2(400, 400), ImGuiCond_FirstUseEver);
		if (ImGui::Begin(windowTitle, &windowIsOpen))
		{
			if (context.editingAsset.HasValue())
			{
				Uid uid = context.editingAsset.GetValue();

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
					if (asset->GetType() == AssetType::Material)
					{
						DrawMaterial(context, asset);
					}
					else if (asset->GetType() == AssetType::Texture)
					{
						DrawTexture(context, asset);
					}
					else
					{
						ImGui::Text("Selected asset is an unknown asset type");
					}
				}
			}
			else // No asset selected
			{
				const char* const text = "No asset selected";
				ImVec2 region = ImGui::GetContentRegionAvail();
				ImVec2 textSize = ImGui::CalcTextSize(text);
				ImVec2 pos((region.x - textSize.x) / 2.0f, (region.y - textSize.y) / 2.0f);
				ImGui::SetCursorPos(pos);
				ImGui::Text(text);
			}
		}

		if (requestFocus)
			ImGui::SetWindowFocus();

		ImGui::End();
	}
}

void AssetView::DrawMaterial(EditorContext& context, const AssetInfo* asset)
{
	if (asset->GetType() != AssetType::Material)
		return;

	MaterialId materialId = materialManager->FindMaterialByUid(asset->GetUid());
	if (materialId == MaterialId::Null)
	{
		ImGui::Text("Material not found in MaterialManager");
		return;
	}

	bool edited = false;

	String& textStore = context.temporaryString;
	textStore.Assign(asset->GetFilename());
	ImGui::Text("%s", textStore.GetCStr());

	ShaderId shaderId = materialManager->GetMaterialShader(materialId);

	textStore.Assign("No shader");
	if (shaderId != ShaderId::Null)
	{
		const ShaderData& shader = shaderManager->GetShaderData(shaderId);
		auto shaderAsset = context.assetLibrary->FindAssetByUid(shader.uid);
		if (shaderAsset != nullptr)
		{
			textStore.Assign(shaderAsset->GetFilename());
		}
	}

	ImGui::InputText("Shader", textStore.GetData(), textStore.GetLength() + 1, ImGuiInputTextFlags_ReadOnly);
	edited |= DrawMaterialShaderDropTarget(context, materialId);

	UniformData& uniforms = materialManager->GetMaterialUniforms(materialId);

	if (ImGui::CollapsingHeader("Properties", ImGuiTreeNodeFlags_DefaultOpen))
	{
		for (auto& uniform : uniforms.GetBufferUniforms())
		{
			edited |= DrawMaterialProperty(context, uniforms, uniform);
		}
	}

	if (edited)
		materialManager->UpdateUniformsToGPU(materialId);
	
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
				float side = ImGui::GetFontSize() * 6.0f;
				ImVec2 size(side, side);

				void* texId = reinterpret_cast<void*>(static_cast<size_t>(texture.textureObject.i));
				ImVec2 uv0(0.0f, 1.0f);
				ImVec2 uv1(1.0f, 0.0f);

				ImGui::Image(texId, size, uv0, uv1);

				edited |= DrawMaterialTextureDropTarget(context, uniforms, texture);
			}

			ImGui::Spacing();
		}
	}

	if (edited)
	{
		// Serialize
		MaterialSerializer serializer(allocator, materialManager, shaderManager, textureManager);

		String serialized(allocator);
		serializer.SerializeToString(materialId, serialized);

		ArrayView<const uint8_t> view(reinterpret_cast<const uint8_t*>(serialized.GetData()), serialized.GetLength());

		if (context.assetLibrary->UpdateAssetContent(asset->GetUid(), view) == false)
		{
			KK_LOG_ERROR("Failed to update asset");
		}
	}
}

bool AssetView::DrawMaterialProperty(EditorContext& context, UniformData& uniforms, const BufferUniform& prop)
{
	bool edited = false;

	String& textStore = context.temporaryString;
	textStore.Assign(prop.name);

	const char* unsupportedPropertyType = nullptr;

	switch (prop.type)
	{
	case UniformDataType::Tex2D:
		unsupportedPropertyType = "Tex2D";
		break;

	case UniformDataType::TexCube:
		unsupportedPropertyType = "TexCube";
		break;

	case UniformDataType::Mat4x4:
		unsupportedPropertyType = "Mat4x4";
		break;

	case UniformDataType::Mat4x4Array:
		unsupportedPropertyType = "Mat4x4Array";
		break;

	case UniformDataType::Mat3x3:
		unsupportedPropertyType = "Mat3x3";
		break;

	case UniformDataType::Mat3x3Array:
		unsupportedPropertyType = "Mat3x3Array";
		break;

	case UniformDataType::Vec4:
	{
		auto value = uniforms.GetValue<Vec4f>(prop);
		if (ImGui::InputFloat4(textStore.GetCStr(), value.ValuePointer()))
		{
			uniforms.SetValue(prop, value);
			edited = true;
		}
		break;
	}

	case UniformDataType::Vec4Array:
		unsupportedPropertyType = "Vec4Array";
		break;

	case UniformDataType::Vec3:
	{
		auto value = uniforms.GetValue<Vec3f>(prop);
		if (ImGui::InputFloat3(textStore.GetCStr(), value.ValuePointer()))
		{
			uniforms.SetValue(prop, value);
			edited = true;
		}
		break;
	}

	case UniformDataType::Vec3Array:
		unsupportedPropertyType = "Vec3Array";
		break;

	case UniformDataType::Vec2:
	{
		auto value = uniforms.GetValue<Vec2f>(prop);
		if (ImGui::InputFloat2(textStore.GetCStr(), value.ValuePointer()))
		{
			uniforms.SetValue(prop, value);
			edited = true;
		}
		break;
	}

	case UniformDataType::Vec2Array:
		unsupportedPropertyType = "Vec2Array";
		break;

	case UniformDataType::Float:
	{
		auto value = uniforms.GetValue<float>(prop);
		if (ImGui::InputFloat(textStore.GetCStr(), &value))
		{
			uniforms.SetValue(prop, value);
			edited = true;
		}
		break;
	}

	case UniformDataType::FloatArray:
		unsupportedPropertyType = "FloatArray";
		break;

	case UniformDataType::Int:
	{
		auto value = uniforms.GetValue<int>(prop);
		if (ImGui::InputInt(textStore.GetCStr(), &value))
		{
			uniforms.SetValue(prop, value);
			edited = true;
		}
		break;
	}

	case UniformDataType::IntArray:
		unsupportedPropertyType = "IntArray";
		break;

	default:
		unsupportedPropertyType = "unknown";
		break;
	}
	
	if (unsupportedPropertyType != nullptr)
		ImGui::Text("Unsupported property type: %s", unsupportedPropertyType);

	ImGui::Spacing();

	return edited;
}

bool AssetView::DrawMaterialShaderDropTarget(EditorContext& context, kokko::MaterialId materialId)
{
	if (ImGui::BeginDragDropTarget())
	{
		const auto* payload = ImGui::AcceptDragDropPayload(EditorConstants::AssetDragDropType);
		if (payload != nullptr && payload->DataSize == sizeof(Uid))
		{
			Uid assetUid;
			std::memcpy(&assetUid, payload->Data, payload->DataSize);

			auto asset = context.assetLibrary->FindAssetByUid(assetUid);
			if (asset != nullptr && asset->GetType() == AssetType::Shader)
			{
				ShaderId shaderId = shaderManager->FindShaderByUid(assetUid);
				if (shaderId != ShaderId::Null)
				{
					materialManager->SetMaterialShader(materialId, shaderId);

					return true;
				}
			}
		}

		ImGui::EndDragDropTarget();
	}

	return false;
}

bool AssetView::DrawMaterialTextureDropTarget(
	EditorContext& context,
	UniformData& uniforms,
	TextureUniform& texture)
{
	if (ImGui::BeginDragDropTarget())
	{
		const auto* payload = ImGui::AcceptDragDropPayload(EditorConstants::AssetDragDropType);
		if (payload != nullptr && payload->DataSize == sizeof(Uid))
		{
			Uid assetUid;
			std::memcpy(&assetUid, payload->Data, payload->DataSize);

			auto asset = context.assetLibrary->FindAssetByUid(assetUid);
			if (asset != nullptr && asset->GetType() == AssetType::Texture)
			{
				TextureId newTexId = textureManager->FindTextureByUid(assetUid);
				if (newTexId != TextureId::Null && newTexId != texture.textureId)
				{
					const TextureData& newTexData = textureManager->GetTextureData(newTexId);

					uniforms.SetTexture(texture, newTexId, newTexData.textureObjectId);

					return true;
				}
			}
		}

		ImGui::EndDragDropTarget();
	}

	return false;
}

void AssetView::DrawTexture(EditorContext& context, const AssetInfo* asset)
{
	if (asset->GetType() != AssetType::Texture)
		return;

	TextureId textureId = textureManager->FindTextureByUid(asset->GetUid());
	if (textureId == TextureId::Null)
	{
		ImGui::Text("Texture not found in TextureManager");
		return;
	}

	context.temporaryString.Assign(asset->GetFilename());
	ImGui::Text("%s", context.temporaryString.GetCStr());

	const TextureData& texture = textureManager->GetTextureData(textureId);

	float side = ImGui::GetFontSize() * 10.0f;
	ImVec2 size(side, side);

	void* texId = reinterpret_cast<void*>(static_cast<size_t>(texture.textureObjectId.i));
	ImVec2 uv0(0.0f, 1.0f);
	ImVec2 uv1(1.0f, 0.0f);

	ImGui::Image(texId, size, uv0, uv1);
}

}
}
