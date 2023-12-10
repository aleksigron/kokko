#pragma once

#include "ryml.hpp"

#include "Core/Core.hpp"
#include "Core/Hash.hpp"

#include "Engine/ComponentSerializer.hpp"
#include "Engine/Entity.hpp"

#include "Rendering/LightManager.hpp"

class ParticleSystem;

namespace kokko
{

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

	virtual void DeserializeComponent(const c4::yml::ConstNodeRef& map, Entity entity) override
	{
		LightType type;

		auto typeNode = map.find_child("type");
		if (typeNode.valid() && typeNode.has_val())
		{
			auto typeStr = typeNode.val();
			uint32_t typeHash = kokko::HashString(typeStr.str, typeStr.len);

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
		auto colorNode = map.find_child("color");
		if (colorNode.valid() && colorNode.has_val())
			colorNode >> color;

		float intensity = -1.0f;
		auto intensityNode = map.find_child("intensity");
		if (intensityNode.valid() && intensityNode.has_val())
			intensityNode >> intensity;

		float radius = -1.0f;
		auto radiusNode = map.find_child("radius");
		if (radiusNode.valid() && radiusNode.has_val())
			radiusNode >> radius;

		float angle = 1.0f;
		auto angleNode = map.find_child("spot_angle");
		if (angleNode.valid() && angleNode.has_val())
			angleNode >> angle;

		bool shadowCasting = false;
		auto shadowNode = map.find_child("cast_shadow");
		if (shadowNode.valid() && shadowNode.has_val())
			shadowNode >> shadowCasting;

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

	virtual void SerializeComponent(c4::yml::NodeRef& componentArray, Entity entity) override
	{
		LightId lightId = lightManager->Lookup(entity);
		if (lightId != LightId::Null)
		{
			ryml::NodeRef componentNode = componentArray.append_child();
			componentNode |= ryml::MAP;
			componentNode[GetComponentTypeKey()] = "light";

			LightType lightType = lightManager->GetLightType(lightId);
			componentNode["type"] << LightManager::GetLightTypeName(lightType);

			Vec3f lightColor = lightManager->GetColor(lightId);
			componentNode["color"] << lightColor;

			float lightIntensity = lightManager->GetIntensity(lightId);
			componentNode["intensity"] << lightIntensity;

			if (lightType != LightType::Directional)
			{
				float radius = lightManager->GetRadius(lightId);
				componentNode["radius"] << radius;
			}

			if (lightType == LightType::Spot)
			{
				float spotAngle = lightManager->GetSpotAngle(lightId);
				componentNode["spot_angle"] << spotAngle;
			}

			bool shadowCasting = lightManager->GetShadowCasting(lightId);
			componentNode["cast_shadow"] << shadowCasting;
		}
	}
};

} // namespace kokko
