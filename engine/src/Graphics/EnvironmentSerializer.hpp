#pragma once

#include "yaml-cpp/yaml.h"

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

	virtual void DeserializeComponent(const YAML::Node& map, Entity entity) override
	{
		YAML::Node textureNode = map["source_texture"];
		YAML::Node intensityNode = map["intensity"];

		if (textureNode.IsDefined() && textureNode.IsScalar())
		{
			const std::string& textureUidStr = textureNode.Scalar();
			auto uidResult = kokko::Uid::FromString(ArrayView(textureUidStr.c_str(), textureUidStr.length()));

			EnvironmentId envId = envSystem->AddComponent(entity);

			if (uidResult.HasValue())
				envSystem->SetEnvironmentTexture(envId, uidResult.GetValue());
			else
				KK_LOG_WARN("EnvironmentSerializer: Parsing source texture UID from string failed");

			if (intensityNode.IsDefined() && intensityNode.IsScalar())
				envSystem->SetIntensity(envId, intensityNode.as<float>());
		}
	}

	virtual void SerializeComponent(YAML::Emitter& out, Entity entity) override
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

			out << YAML::BeginMap;
			out << YAML::Key << GetComponentTypeKey() << YAML::Value << "environment";
			out << YAML::Key << "source_texture" << YAML::Value << textureUidStr;
			out << YAML::Key << "intensity" << YAML::Value << intensity;
			out << YAML::EndMap;
		}
	}
};

} // namespace kokko
