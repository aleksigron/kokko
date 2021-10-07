#pragma once

#include "yaml-cpp/yaml.h"

#include "Core/Hash.hpp"

#include "Engine/ComponentSerializer.hpp"
#include "Engine/Entity.hpp"

#include "Graphics/TerrainSystem.hpp"

#include "Resources/YamlCustomTypes.hpp"

class TerrainSerializer final : public ComponentSerializer
{
public:
	TerrainSerializer(TerrainSystem* terrainSystem) :
		terrainSystem(terrainSystem)
	{
	}

	virtual uint32_t GetComponentTypeNameHash() override
	{
		return "terrain"_hash;
	}

	virtual void DeserializeComponent(const YAML::Node& map, Entity entity) override
	{
		TerrainParameters terrain;

		YAML::Node sizeNode = map["terrain_size"];
		if (sizeNode.IsDefined() && sizeNode.IsScalar())
			terrain.terrainSize = sizeNode.as<float>();

		YAML::Node resNode = map["terrain_resolution"];
		if (resNode.IsDefined() && resNode.IsScalar())
			terrain.terrainResolution = resNode.as<int>();

		YAML::Node scaleNode = map["texture_scale"];
		if (scaleNode.IsDefined() && scaleNode.IsSequence())
			terrain.textureScale = scaleNode.as<Vec2f>();

		YAML::Node minNode = map["min_height"];
		if (minNode.IsDefined() && minNode.IsScalar())
			terrain.minHeight = minNode.as<float>();

		YAML::Node maxNode = map["max_height"];
		if (maxNode.IsDefined() && maxNode.IsScalar())
			terrain.maxHeight = maxNode.as<float>();

		TerrainId id = terrainSystem->AddTerrain(entity, terrain);
	}

	virtual void SerializeComponent(YAML::Emitter& out, Entity entity) override
	{
		TerrainId terrainId = terrainSystem->Lookup(entity);
		if (terrainId != TerrainId::Null)
		{
			out << YAML::BeginMap;
			out << YAML::Key << GetComponentTypeKey() << YAML::Value << "terrain";

			int resolution = terrainSystem->GetResolution(terrainId);
			float size = terrainSystem->GetSize(terrainId);
			float minHeight = terrainSystem->GetMinHeight(terrainId);
			float maxHeight = terrainSystem->GetMaxHeight(terrainId);
			Vec2f textureScale = terrainSystem->GetTextureScale(terrainId);

			out << YAML::Key << "terrain_size" << YAML::Value << size;
			out << YAML::Key << "terrain_resolution" << YAML::Value << resolution;
			out << YAML::Key << "texture_scale" << YAML::Value << textureScale;
			out << YAML::Key << "min_height" << YAML::Value << minHeight;
			out << YAML::Key << "max_height" << YAML::Value << maxHeight;

			out << YAML::EndMap;
		}
	}

private:
	TerrainSystem* terrainSystem;
};
