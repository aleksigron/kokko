#pragma once

#include <cassert>

#include "yaml-cpp/yaml.h"

#include "Core/Hash.hpp"

#include "Engine/ComponentSerializer.hpp"
#include "Engine/Entity.hpp"

#include "Rendering/MeshComponentSystem.hpp"

#include "Resources/MaterialManager.hpp"
#include "Resources/MeshManager.hpp"
#include "Resources/MeshUid.hpp"
#include "Resources/ModelManager.hpp"
#include "Resources/ResourceManagers.hpp"

namespace kokko
{

class MeshComponentSerializer final : public ComponentSerializer
{
private:
	MeshComponentSystem* meshComponentSystem;
	ResourceManagers res;

public:
	MeshComponentSerializer(
		MeshComponentSystem* meshComponentSystem,
		const ResourceManagers& resourceManagers) :
		meshComponentSystem(meshComponentSystem),
		res(resourceManagers)
	{
	}

	virtual uint32_t GetComponentTypeNameHash() override
	{
		return "mesh"_hash;
	}

	virtual void DeserializeComponent(const YAML::Node& map, Entity entity) override
	{
		YAML::Node meshNode = map["mesh"];
		YAML::Node materialNode = map["material"];

		if (meshNode.IsDefined() && meshNode.IsScalar() &&
			materialNode.IsDefined() && materialNode.IsScalar())
		{
			MeshComponentId componentId = meshComponentSystem->AddComponent(entity);

			const std::string& meshUidStr = meshNode.Scalar();
			auto meshUidOpt = MeshUid::FromString(ArrayView(meshUidStr.c_str(), meshUidStr.length()));

			if (meshUidOpt.HasValue())
			{
				MeshUid meshUid = meshUidOpt.GetValue();
				ModelId modelId = res.modelManager->FindModelByUid(meshUid.modelUid);

				if (modelId != ModelId::Null)
				{
					auto modelMeshes = res.modelManager->GetModelMeshes(modelId);

					if (modelMeshes.GetCount() > meshUid.meshIndex)
						meshComponentSystem->SetMeshId(componentId, modelMeshes[meshUid.meshIndex].meshId);
				}
			}

			const std::string& matUidStr = materialNode.Scalar();
			auto materialUid = Uid::FromString(ArrayView(matUidStr.c_str(), matUidStr.length()));

			if (materialUid.HasValue())
			{
				MaterialId materialId = res.materialManager->FindMaterialByUid(materialUid.GetValue());

				// We don't care if the material isn't found
				// Renderer will use fallback material if Material::Null is specified

				TransparencyType transparency = TransparencyType::Opaque;
				if (materialId != MaterialId::Null)
					transparency = res.materialManager->GetMaterialTransparency(materialId);

				meshComponentSystem->SetMaterial(componentId, materialId, transparency);
			}
		}
	}

	virtual void SerializeComponent(YAML::Emitter& out, Entity entity) override
	{
		MeshComponentId componentId = meshComponentSystem->Lookup(entity);
		if (componentId != MeshComponentId::Null)
		{
			MeshId meshId = meshComponentSystem->GetMeshId(componentId);

			Optional<Uid> modelUidOpt;
			if (meshId != MeshId::Null)
				modelUidOpt = res.meshManager->GetUid(meshId);

			char meshUidStr[MeshUid::StringLength + 1];
			meshUidStr[0] = '\0';

			if (modelUidOpt.HasValue())
			{
				ModelId modelId = res.modelManager->FindModelByUid(modelUidOpt.GetValue());

				if (modelId != ModelId::Null)
				{
					auto modelMeshes = res.modelManager->GetModelMeshes(modelId);
					auto modelMesh = modelMeshes.FindIf(
						[meshId](auto& val) { return val.meshId == meshId; });

					if (modelMesh != nullptr)
					{
						MeshUid meshUid;
						meshUid.modelUid = modelUidOpt.GetValue();
						meshUid.meshIndex = static_cast<uint32_t>(modelMesh - modelMeshes.GetData());

						meshUid.WriteTo(meshUidStr);
						meshUidStr[MeshUid::StringLength] = '\0';
					}
				}
			}

			char materialUidStr[Uid::StringLength + 1];
			materialUidStr[0] = '\0';

			MaterialId materialId = meshComponentSystem->GetMaterialId(componentId);
			if (materialId != MaterialId::Null)
			{
				Uid materialUid = res.materialManager->GetMaterialUid(materialId);
				materialUid.WriteTo(materialUidStr);
				materialUidStr[Uid::StringLength] = '\0';
			}

			out << YAML::BeginMap;
			out << YAML::Key << GetComponentTypeKey() << YAML::Value << "mesh";
			out << YAML::Key << "mesh" << YAML::Value << meshUidStr;
			out << YAML::Key << "material" << YAML::Value << materialUidStr;
			out << YAML::EndMap;
		}
	}
};

} // namespace kokko
