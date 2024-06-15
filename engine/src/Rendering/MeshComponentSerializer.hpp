#pragma once

#include <cassert>

#include "ryml.hpp"

#include "Core/Hash.hpp"

#include "Engine/ComponentSerializer.hpp"
#include "Engine/Entity.hpp"

#include "Rendering/MeshComponentSystem.hpp"

#include "Resources/MaterialManager.hpp"
#include "Resources/ModelManager.hpp"
#include "Resources/MeshId.hpp"
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

	virtual void DeserializeComponent(const c4::yml::ConstNodeRef& map, Entity entity) override
	{
		auto meshNode = map.find_child("mesh");
		auto materialNode = map.find_child("material");

		if (meshNode.valid() && meshNode.has_val() &&
			materialNode.valid() && (materialNode.has_val() || materialNode.is_seq()))
		{
			MeshComponentId componentId = meshComponentSystem->AddComponent(entity);

			auto meshUidStr = meshNode.val();
			auto meshUidOpt = MeshUid::FromString(ArrayView(meshUidStr.str, meshUidStr.len));

			if (meshUidOpt.HasValue())
			{
				MeshUid meshUid = meshUidOpt.GetValue();
				ModelId modelId = res.modelManager->FindModelByUid(meshUid.modelUid);

				if (modelId != ModelId::Null)
				{
					auto modelMeshes = res.modelManager->GetModelMeshes(modelId);

					if (modelMeshes.GetCount() > meshUid.meshIndex)
					{
						MeshId newMeshId = MeshId{ modelId, meshUid.meshIndex };
						uint32_t partCount = modelMeshes[meshUid.meshIndex].partCount;
						meshComponentSystem->SetMesh(componentId, newMeshId, partCount);
					}
				}
			}

			auto parseAndSetMaterial = [&](ryml::ConstNodeRef node, uint32_t partIndex)
			{
				auto matUidStr = node.val();
				auto materialUid = Uid::FromString(ArrayView(matUidStr.str, matUidStr.len));

				if (materialUid.HasValue())
				{
					MaterialId materialId = res.materialManager->FindMaterialByUid(materialUid.GetValue());

					// We don't care if the material isn't found
					// Renderer will use fallback material if Material::Null is specified

					TransparencyType transparency = TransparencyType::Opaque;
					if (materialId != MaterialId::Null)
						transparency = res.materialManager->GetMaterialTransparency(materialId);

					meshComponentSystem->SetMaterial(componentId, partIndex, materialId, transparency);
				}
			};

			if (materialNode.is_seq())
			{
				uint32_t index = 0;
				for (auto node : materialNode)
				{
					parseAndSetMaterial(node, index);
					++index;
				}
			}
			else
			{

				parseAndSetMaterial(materialNode, 0);
			}
		}
	}

	virtual void SerializeComponent(c4::yml::NodeRef& componentArray, Entity entity) override
	{
		MeshComponentId componentId = meshComponentSystem->Lookup(entity);
		if (componentId != MeshComponentId::Null)
		{
			MeshId meshId = meshComponentSystem->GetMeshId(componentId);

			bool validMesh = false;
			Optional<Uid> modelUidOpt;
			if (meshId != MeshId::Null)
				modelUidOpt = res.modelManager->GetModelUid(meshId.modelId);

			char meshUidStr[MeshUid::StringLength + 1];
			meshUidStr[0] = '\0';

			if (modelUidOpt.HasValue())
			{
				ModelId modelId = res.modelManager->FindModelByUid(modelUidOpt.GetValue());

				if (modelId != ModelId::Null)
				{
					MeshUid meshUid;
					meshUid.modelUid = modelUidOpt.GetValue();
					meshUid.meshIndex = meshId.meshIndex;

					meshUid.WriteTo(meshUidStr);
					meshUidStr[MeshUid::StringLength] = '\0';

					validMesh = true;
				}
			}

			ryml::NodeRef componentNode = componentArray.append_child();
			componentNode |= ryml::MAP;
			componentNode[GetComponentTypeKey()] = "mesh";

			if (validMesh)
				componentNode["mesh"] << meshUidStr;

			char materialUidStr[Uid::StringLength + 1];
			materialUidStr[0] = '\0';

			auto materials = meshComponentSystem->GetMaterialIds(componentId);
			if (materials.GetCount() != 0)
			{
				auto materialNode = componentNode.append_child(ryml::NodeInit(ryml::NodeType_e::SEQ, "material"));
			
				for (auto materialId : materials)
				{
					auto materialItem = materialNode.append_child();

					if (materialId != MaterialId::Null)
					{
						Uid materialUid = res.materialManager->GetMaterialUid(materialId);
						materialUid.WriteTo(materialUidStr);
						materialUidStr[Uid::StringLength] = '\0';

						materialItem << materialUidStr;
					}
					else
					{
						materialItem << "";
					}
				}
			}
		}
	}
};

} // namespace kokko
