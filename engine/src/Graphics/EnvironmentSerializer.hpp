#pragma once

#include "ryml.hpp"

#include "Core/Hash.hpp"

#include "Engine/ComponentSerializer.hpp"
#include "Engine/Entity.hpp"

#include "Graphics/EnvironmentSystem.hpp"

namespace kokko
{

class EnvironmentSerializer final : public ComponentSerializer
{
private:
	EnvironmentSystem* envSystem;

public:
	EnvironmentSerializer(EnvironmentSystem* envSystem) :
		envSystem(envSystem)
	{
	}

	virtual uint32_t GetComponentTypeNameHash() override
	{
		return "environment"_hash;
	}

	virtual void DeserializeComponent(const c4::yml::ConstNodeRef& map, Entity entity) override
	{
		auto textureNode = map.find_child("source_texture");
		auto intensityNode = map.find_child("intensity");

		if (textureNode.valid() && textureNode.has_val())
		{
			auto textureUidStr = textureNode.val();
			auto uidResult = kokko::Uid::FromString(ArrayView(textureUidStr.str, textureUidStr.len));

			EnvironmentId envId = envSystem->AddComponent(entity);

			if (uidResult.HasValue())
				envSystem->SetEnvironmentTexture(envId, uidResult.GetValue());
			else
				KK_LOG_WARN("EnvironmentSerializer: Parsing source texture UID from string failed");

			if (intensityNode.valid() && intensityNode.has_val() && intensityNode.val().is_real())
			{
				float intensity;
				intensityNode >> intensity;
				envSystem->SetIntensity(envId, intensity);
			}
		}
	}

	virtual void SerializeComponent(c4::yml::NodeRef& componentArray, Entity entity) override
	{
		EnvironmentId envId = envSystem->Lookup(entity);
		if (envId != EnvironmentId::Null)
		{
			char textureUidBuf[Uid::StringLength + 1];
			const char* textureUidStr = "";

			if (Optional<Uid> textureUidOpt = envSystem->GetSourceTextureUid(envId))
			{
				textureUidOpt.GetValue().WriteTo(textureUidBuf);
				textureUidBuf[Uid::StringLength] = '\0';
				textureUidStr = textureUidBuf;
			}

			float intensity = envSystem->GetIntensity(envId);

			ryml::NodeRef componentNode = componentArray.append_child();
			componentNode |= ryml::MAP;
			componentNode[GetComponentTypeKey()] = "environment";

			componentNode["source_texture"] << textureUidStr;
			componentNode["intensity"] << intensity;
		}
	}
};

} // namespace kokko
