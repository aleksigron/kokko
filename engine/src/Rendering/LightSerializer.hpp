#pragma once

#include "yaml-cpp/yaml.h"

#include "Core/Core.hpp"
#include "Core/Hash.hpp"

#include "Engine/ComponentSerializer.hpp"
#include "Engine/Entity.hpp"

#include "Rendering/LightManager.hpp"

class ParticleSystem;

class LightSerializer final : public ComponentSerializer
{
private:
	LightManager* lightManager;

public:
	LightSerializer(LightManager* lightManager) :
		lightManager(lightManager)
	{
	}

	virtual uint32_t GetComponentTypeNameHash() override
	{
		return "light"_hash;
	}

	virtual void DeserializeComponent(const YAML::Node& map, Entity entity) override
	{
		LightType type;

		YAML::Node typeNode = map["type"];
		if (typeNode.IsDefined() && typeNode.IsScalar())
		{
			const std::string& typeStr = typeNode.Scalar();
			uint32_t typeHash = kokko::HashString(typeStr.data(), typeStr.size());

			switch (typeHash)
			{
			case "directional"_hash:
				type = LightType::Directional;
				break;

			case "point"_hash:
				type = LightType::Point;
				break;

			case "spot"_hash:
				type = LightType::Spot;
				break;

			default:
				KK_LOG_ERROR("LevelLoader: Unknown light type");
				return;
			}
		}
		else
		{
			KK_LOG_ERROR("LevelLoader: Light type not specified");
			return;
		}


		Vec3f color(1.0f, 1.0f, 1.0f);
		YAML::Node colorNode = map["color"];
		if (colorNode.IsDefined() && colorNode.IsSequence())
			color = colorNode.as<Vec3f>();

		float intensity = -1.0f;
		YAML::Node intensityNode = map["intensity"];
		if (intensityNode.IsDefined() && intensityNode.IsScalar())
			intensity = intensityNode.as<float>();

		float radius = -1.0f;
		YAML::Node radiusNode = map["radius"];
		if (radiusNode.IsDefined() && radiusNode.IsScalar())
			radius = radiusNode.as<float>();

		float angle = 1.0f;
		YAML::Node angleNode = map["spot_angle"];
		if (angleNode.IsDefined() && angleNode.IsScalar())
			angle = angleNode.as<float>();

		bool shadowCasting = false;
		YAML::Node shadowNode = map["cast_shadow"];
		if (shadowNode.IsDefined() && shadowNode.IsScalar())
			shadowCasting = shadowNode.as<bool>();

		LightId lightId = lightManager->AddLight(entity);
		lightManager->SetLightType(lightId, type);
		lightManager->SetColor(lightId, color);
		lightManager->SetIntensity(lightId, intensity >= 0.0f ? intensity : 1.0f);

		if (type != LightType::Directional)
		{
			if (radius > 0.0f)
				lightManager->SetRadius(lightId, radius);
			else
				lightManager->SetRadiusFromColor(lightId);
		}
		else
			lightManager->SetRadius(lightId, 1.0f);

		lightManager->SetSpotAngle(lightId, angle);
		lightManager->SetShadowCasting(lightId, shadowCasting);
	}

	virtual void SerializeComponent(YAML::Emitter& out, Entity entity) override
	{
		LightId lightId = lightManager->Lookup(entity);
		if (lightId != LightId::Null)
		{
			out << YAML::BeginMap;
			out << YAML::Key << GetComponentTypeKey() << YAML::Value << "light";

			LightType lightType = lightManager->GetLightType(lightId);
			out << YAML::Key << "type" << YAML::Value << LightManager::GetLightTypeName(lightType);

			Vec3f lightColor = lightManager->GetColor(lightId);
			out << YAML::Key << "color" << YAML::Value << lightColor;

			float lightIntensity = lightManager->GetIntensity(lightId);
			out << YAML::Key << "intensity" << YAML::Value << lightIntensity;

			if (lightType != LightType::Directional)
			{
				float radius = lightManager->GetRadius(lightId);
				out << YAML::Key << "radius" << YAML::Value << radius;
			}

			if (lightType == LightType::Spot)
			{
				float spotAngle = lightManager->GetSpotAngle(lightId);
				out << YAML::Key << "spot_angle" << YAML::Value << spotAngle;
			}

			bool shadowCasting = lightManager->GetShadowCasting(lightId);
			out << YAML::Key << "cast_shadow" << YAML::Value << shadowCasting;

			out << YAML::EndMap;
		}
	}
};
