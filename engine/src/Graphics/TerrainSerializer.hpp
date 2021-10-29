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
	TerrainSerializer(kokko::TerrainSystem* terrainSystem) :
		terrainSystem(terrainSystem)
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

			out << YAML::EndMap;
		}
	}

private:
	kokko::TerrainSystem* terrainSystem;
};
