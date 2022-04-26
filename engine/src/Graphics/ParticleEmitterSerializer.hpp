#pragma once

#include "yaml-cpp/yaml.h"

#include "Core/Hash.hpp"

#include "Engine/ComponentSerializer.hpp"
#include "Engine/Entity.hpp"

#include "Graphics/ParticleSystem.hpp"

class ParticleEmitterSerializer final : public ComponentSerializer
{
public:
	ParticleEmitterSerializer(kokko::ParticleSystem* particleSystem) :
		particleSystem(particleSystem)
	{
	}

	virtual uint32_t GetComponentTypeNameHash() override
	{
		return "particle"_hash;
	}

	virtual void DeserializeComponent(const YAML::Node& map, Entity entity) override
	{
		kokko::ParticleEmitterId id = particleSystem->AddEmitter(entity);

		float emitRate = 0.0f;

		YAML::Node rateNode = map["emit_rate"];
		if (rateNode.IsDefined() && rateNode.IsScalar())
			emitRate = rateNode.as<float>();

		particleSystem->SetEmitRate(id, emitRate);
	}

	virtual void SerializeComponent(YAML::Emitter& out, Entity entity) override
	{
		kokko::ParticleEmitterId emitterId = particleSystem->Lookup(entity);
		if (emitterId != kokko::ParticleEmitterId::Null)
		{
			out << YAML::BeginMap;
			out << YAML::Key << GetComponentTypeKey() << YAML::Value << "particle";

			float emitRate = particleSystem->GetEmitRate(emitterId);
			out << YAML::Key << "emit_rate" << YAML::Value << emitRate;

			out << YAML::EndMap;
		}
	}

private:
	kokko::ParticleSystem* particleSystem;
};
