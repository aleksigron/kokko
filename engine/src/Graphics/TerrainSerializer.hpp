#pragma once

#include "ryml.hpp"

#include "Core/Hash.hpp"
#include "Core/Uid.hpp"

#include "Engine/ComponentSerializer.hpp"
#include "Engine/Entity.hpp"

#include "Graphics/TerrainSystem.hpp"

#include "Resources/TextureManager.hpp"
#include "Resources/YamlCustomTypes.hpp"

namespace kokko
{

class TerrainSerializer final : public ComponentSerializer
{
public:
	TerrainSerializer(TerrainSystem* terrainSystem, TextureManager* textureManager) :
		terrainSystem(terrainSystem),
		textureManager(textureManager)
	{
	}

	virtual uint32_t GetComponentTypeNameHash() override
	{
		return "terrain"_hash;
	}

	virtual void DeserializeComponent(const c4::yml::ConstNodeRef& map, Entity entity) override
	{
		TerrainId id = terrainSystem->AddTerrain(entity);

		auto sizeNode = map.find_child("terrain_size");
		if (sizeNode.valid() && sizeNode.has_val() && sizeNode.val().is_real())
		{
			float size;
			sizeNode >> size;
			terrainSystem->SetSize(id, size);
		}

		auto scaleNode = map.find_child("texture_scale");
		if (scaleNode.valid() && scaleNode.has_val())
		{
			Vec2f textureScale;
			scaleNode >> textureScale;
			terrainSystem->SetTextureScale(id, textureScale);
		}

		auto minNode = map.find_child("terrain_bottom");
		if (minNode.valid() && minNode.has_val() && minNode.val().is_real())
		{
			float bottom;
			minNode >> bottom;
			terrainSystem->SetBottom(id, bottom);
		}

		auto heightNode = map.find_child("terrain_height");
		if (heightNode.valid() && heightNode.has_val() && heightNode.val().is_real())
		{
			float height;
			heightNode >> height;
			terrainSystem->SetHeight(id, height);
		}

		auto heightTexNode = map.find_child("height_texture");
		if (heightTexNode.valid() && heightTexNode.has_val())
		{
			auto uidSubstr = heightTexNode.val();
			auto uidOpt = Uid::FromString(ArrayView(uidSubstr.str, uidSubstr.len));
			if (uidOpt.HasValue())
			{
				terrainSystem->SetHeightTexture(id, uidOpt.GetValue());
			}
		}

		auto albedoNode = map.find_child("albedo_texture");
		if (albedoNode.valid() && albedoNode.has_val())
		{
			auto uidSubstr = albedoNode.val();
			auto uidOpt = Uid::FromString(ArrayView(uidSubstr.str, uidSubstr.len));
			if (uidOpt.HasValue())
			{
				TextureId textureId = textureManager->FindTextureByUid(uidOpt.GetValue());

				if (textureId != TextureId::Null)
				{
					const TextureData& texture = textureManager->GetTextureData(textureId);
					terrainSystem->SetAlbedoTexture(id, textureId, texture.textureObjectId);
				}
			}
		}

		auto roughnessNode = map.find_child("roughness_texture");
		if (roughnessNode.valid() && roughnessNode.has_val())
		{
			auto uidSubstr = roughnessNode.val();
			auto uidOpt = Uid::FromString(ArrayView(uidSubstr.str, uidSubstr.len));
			if (uidOpt.HasValue())
			{
				TextureId textureId = textureManager->FindTextureByUid(uidOpt.GetValue());

				if (textureId != TextureId::Null)
				{
					const TextureData& texture = textureManager->GetTextureData(textureId);
					terrainSystem->SetRoughnessTexture(id, textureId, texture.textureObjectId);
				}
			}
		}
	}

	virtual void SerializeComponent(c4::yml::NodeRef& componentArray, Entity entity) override
	{
		TerrainId terrainId = terrainSystem->Lookup(entity);
		if (terrainId != TerrainId::Null)
		{
			ryml::NodeRef componentNode = componentArray.append_child();
			componentNode |= ryml::MAP;
			componentNode[GetComponentTypeKey()] = "terrain";

			float size = terrainSystem->GetSize(terrainId);
			float heightOrigin = terrainSystem->GetBottom(terrainId);
			float heightRange = terrainSystem->GetHeight(terrainId);
			Vec2f textureScale = terrainSystem->GetTextureScale(terrainId);

			componentNode["terrain_size"] << size;
			componentNode["texture_scale"] << textureScale;
			componentNode["terrain_bottom"] << heightOrigin;
			componentNode["terrain_height"] << heightRange;

			char uidStr[Uid::StringLength + 1];

			if (auto heightTexture = terrainSystem->GetHeightTexture(terrainId))
			{
				heightTexture.GetValue().WriteTo(ArrayView(uidStr));
				uidStr[Uid::StringLength] = '\0';
				componentNode["height_texture"] << uidStr;
			}

			TextureId albedoTextureId = terrainSystem->GetAlbedoTextureId(terrainId);
			if (albedoTextureId != TextureId::Null)
			{
				const TextureData& texture = textureManager->GetTextureData(albedoTextureId);
				texture.uid.WriteTo(ArrayView(uidStr));
				uidStr[Uid::StringLength] = '\0';
				componentNode["albedo_texture"] << uidStr;
			}

			TextureId roughnessTextureId = terrainSystem->GetRoughnessTextureId(terrainId);
			if (roughnessTextureId != TextureId::Null)
			{
				const TextureData& texture = textureManager->GetTextureData(roughnessTextureId);
				texture.uid.WriteTo(ArrayView(uidStr));
				uidStr[Uid::StringLength] = '\0';
				componentNode["roughness_texture"] << uidStr;
			}
		}
	}

private:
	TerrainSystem* terrainSystem;
	TextureManager* textureManager;
};

} // namespace kokko
