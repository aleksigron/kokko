#include "AssetView.hpp"

#include "imgui.h"

#include "Core/Core.hpp"

#include "Rendering/Uniform.hpp"
#include "Rendering/UniformData.hpp"

#include "Resources/MaterialManager.hpp"
#include "Resources/MaterialSerializer.hpp"
#include "Resources/ShaderManager.hpp"
#include "Resources/TextureManager.hpp"

#include "AssetLibrary.hpp"
#include "EditorConstants.hpp"
#include "EditorContext.hpp"

namespace kokko
{
namespace editor
{

AssetView::AssetView(Allocator* allocator) :
	EditorWindow("Asset"),
	allocator(allocator),
	materialManager(nullptr),
	shaderManager(nullptr),
	textureManager(nullptr),
	textStore(allocator)
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
					if (asset->type == AssetType::Material)
					{
						DrawMaterial(context, asset);
					}
					else if (asset->type == AssetType::Texture)
					{
						DrawTexture(context, asset);
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

void AssetView::DrawMaterial(EditorContext& context, const AssetInfo* asset)
{
	if (asset->type != AssetType::Material)
		return;

	MaterialId materialId = materialManager->FindMaterialByUid(asset->uid);
	if (materialId == MaterialId::Null)
	{
		ImGui::Text("Material not found in MaterialManager");
		return;
	}

	bool edited = false;

	ImGui::Text("%s", asset->filePath.GetCStr());

	ShaderId shaderId = materialManager->GetMaterialShader(materialId);

	textStore.Assign("No shader");
	if (shaderId != ShaderId::Null)
	{
		const ShaderData& shader = shaderManager->GetShaderData(shaderId);
		auto shaderAsset = context.assetLibrary->FindAssetByUid(shader.uid);
		if (shaderAsset != nullptr)
		{
			textStore = shaderAsset->filePath;
		}
	}

	ImGui::InputText("Shader", textStore.GetData(), textStore.GetLength() + 1, ImGuiInputTextFlags_ReadOnly);
	edited |= DrawMaterialShaderDropTarget(context, materialId);

	UniformData& uniforms = materialManager->GetMaterialUniforms(materialId);

	if (ImGui::CollapsingHeader("Properties", ImGuiTreeNodeFlags_DefaultOpen))
	{
		for (auto& uniform : uniforms.GetBufferUniforms())
		{
			edited |= DrawMaterialProperty(uniforms, uniform);
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

				void* texId = reinterpret_cast<void*>(static_cast<size_t>(texture.textureObject));
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

		String serializedString(allocator);
		serializer.SerializeToString(materialId, serializedString);

		ArrayView<const char> view(serializedString.GetData(), serializedString.GetLength());

		if (context.assetLibrary->UpdateAssetContent(asset->uid, view) == false)
		{
			KK_LOG_ERROR("Failed to update asset");
		}
	}
}

bool AssetView::DrawMaterialProperty(UniformData& uniforms, const BufferUniform& prop)
{
	bool edited = false;

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

bool AssetView::DrawMaterialShaderDropTarget(EditorContext& context, MaterialId materialId)
{
	if (ImGui::BeginDragDropTarget())
	{
		const auto* payload = ImGui::AcceptDragDropPayload(EditorConstants::AssetDragDropType);
		if (payload != nullptr && payload->DataSize == sizeof(Uid))
		{
			Uid assetUid;
			std::memcpy(&assetUid, payload->Data, payload->DataSize);

			auto asset = context.assetLibrary->FindAssetByUid(assetUid);
			if (asset != nullptr && asset->type == AssetType::Shader)
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
			if (asset != nullptr && asset->type == AssetType::Texture)
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
	if (asset->type != AssetType::Texture)
		return;

	TextureId textureId = textureManager->FindTextureByUid(asset->uid);
	if (textureId == TextureId::Null)
	{
		ImGui::Text("Texture not found in TextureManager");
		return;
	}

	ImGui::Text("%s", asset->filePath.GetCStr());

	const TextureData& texture = textureManager->GetTextureData(textureId);

	float side = ImGui::GetFontSize() * 10.0f;
	ImVec2 size(side, side);

	void* texId = reinterpret_cast<void*>(static_cast<size_t>(texture.textureObjectId));
	ImVec2 uv0(0.0f, 1.0f);
	ImVec2 uv1(1.0f, 0.0f);

	ImGui::Image(texId, size, uv0, uv1);
}

}
}
