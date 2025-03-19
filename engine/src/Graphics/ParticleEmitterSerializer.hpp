#pragma once

#include "ryml.hpp"

#include "Core/Hash.hpp"

#include "Engine/ComponentSerializer.hpp"
#include "Engine/Entity.hpp"

#include "Graphics/ParticleSystem.hpp"

namespace kokko
{

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

	virtual void DeserializeComponent(const c4::yml::ConstNodeRef& map, Entity entity) override
	{
		kokko::ParticleEmitterId id = particleSystem->AddEmitter(entity);

		float emitRate = 0.0f;

		auto rateNode = map.find_child("emit_rate");
		if (rateNode.valid() && rateNode.has_val() && rateNode.val().is_real())
			rateNode >> emitRate;

		particleSystem->SetEmitRate(id, emitRate);
	}

	virtual void SerializeComponent(c4::yml::NodeRef& componentArray, Entity entity) override
	{
		kokko::ParticleEmitterId emitterId = particleSystem->Lookup(entity);
		if (emitterId != kokko::ParticleEmitterId::Null)
		{
			ryml::NodeRef componentNode = componentArray.append_child();
			componentNode |= ryml::MAP;
			componentNode[GetComponentTypeKey()] = "particle";

			float emitRate = particleSystem->GetEmitRate(emitterId);
			componentNode["emit_rate"] << emitRate;
		}
	}

private:
	kokko::ParticleSystem* particleSystem;
};

} // namespace kokko
