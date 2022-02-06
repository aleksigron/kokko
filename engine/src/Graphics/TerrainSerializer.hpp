#pragma once

#include "yaml-cpp/yaml.h"

#include "Core/Hash.hpp"
#include "Core/Uid.hpp"

#include "Engine/ComponentSerializer.hpp"
#include "Engine/Entity.hpp"

#include "Graphics/TerrainSystem.hpp"

#include "Resources/TextureManager.hpp"
#include "Resources/YamlCustomTypes.hpp"

class TerrainSerializer final : public ComponentSerializer
{
public:
	TerrainSerializer(kokko::TerrainSystem* terrainSystem, TextureManager* textureManager) :
		terrainSystem(terrainSystem),
		textureManager(textureManager)
	{
	}

	virtual uint32_t GetComponentTypeNameHash() override
	{
		return "terrain"_hash;
	}

	virtual void DeserializeComponent(const YAML::Node& map, Entity entity) override
	{
		kokko::TerrainParameters terrain;

		YAML::Node sizeNode = map["terrain_size"];
		if (sizeNode.IsDefined() && sizeNode.IsScalar())
			terrain.terrainSize = sizeNode.as<float>();

		YAML::Node scaleNode = map["texture_scale"];
		if (scaleNode.IsDefined() && scaleNode.IsSequence())
			terrain.textureScale = scaleNode.as<Vec2f>();

		YAML::Node minNode = map["terrain_bottom"];
		if (minNode.IsDefined() && minNode.IsScalar())
			terrain.heightOrigin = minNode.as<float>();

		YAML::Node maxNode = map["terrain_height"];
		if (maxNode.IsDefined() && maxNode.IsScalar())
			terrain.heightRange = maxNode.as<float>();

		kokko::TerrainId id = terrainSystem->AddTerrain(entity, terrain);

		YAML::Node albedoNode = map["albedo_texture"];
		if (albedoNode.IsDefined() && albedoNode.IsScalar())
		{
			const std::string& uidStr = albedoNode.Scalar();
			auto uidOpt = kokko::Uid::FromString(ArrayView(uidStr.c_str(), uidStr.length()));
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

		YAML::Node roughnessNode = map["roughness_texture"];
		if (roughnessNode.IsDefined() && roughnessNode.IsScalar())
		{
			const std::string& uidStr = roughnessNode.Scalar();
			auto uidOpt = kokko::Uid::FromString(ArrayView(uidStr.c_str(), uidStr.length()));
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

	virtual void SerializeComponent(YAML::Emitter& out, Entity entity) override
	{
		kokko::TerrainId terrainId = terrainSystem->Lookup(entity);
		if (terrainId != kokko::TerrainId::Null)
		{
			out << YAML::BeginMap;
			out << YAML::Key << GetComponentTypeKey() << YAML::Value << "terrain";

			float size = terrainSystem->GetSize(terrainId);
			float heightOrigin = terrainSystem->GetBottom(terrainId);
			float heightRange = terrainSystem->GetHeight(terrainId);
			Vec2f textureScale = terrainSystem->GetTextureScale(terrainId);

			out << YAML::Key << "terrain_size" << YAML::Value << size;
			out << YAML::Key << "texture_scale" << YAML::Value << textureScale;
			out << YAML::Key << "terrain_bottom" << YAML::Value << heightOrigin;
			out << YAML::Key << "terrain_height" << YAML::Value << heightRange;

			char uidStr[kokko::Uid::StringLength + 1];

			TextureId albedoTextureId = terrainSystem->GetAlbedoTextureId(terrainId);
			if (albedoTextureId != TextureId::Null)
			{
				const TextureData& texture = textureManager->GetTextureData(albedoTextureId);
				texture.uid.WriteTo(ArrayView(uidStr));
				uidStr[kokko::Uid::StringLength] = '\0';
				out << YAML::Key << "albedo_texture" << YAML::Value << uidStr;
			}

			TextureId roughnessTextureId = terrainSystem->GetRoughnessTextureId(terrainId);
			if (roughnessTextureId != TextureId::Null)
			{
				const TextureData& texture = textureManager->GetTextureData(roughnessTextureId);
				texture.uid.WriteTo(ArrayView(uidStr));
				uidStr[kokko::Uid::StringLength] = '\0';
				out << YAML::Key << "roughness_texture" << YAML::Value << uidStr;
			}

			out << YAML::EndMap;
		}
	}

private:
	kokko::TerrainSystem* terrainSystem;
	TextureManager* textureManager;
};
