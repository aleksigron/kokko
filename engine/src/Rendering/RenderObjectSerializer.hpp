#pragma once

#include <cassert>

#include "yaml-cpp/yaml.h"

#include "Core/Hash.hpp"

#include "Engine/ComponentSerializer.hpp"
#include "Engine/Entity.hpp"

#include "Rendering/Renderer.hpp"

#include "Resources/MaterialManager.hpp"
#include "Resources/MeshManager.hpp"
#include "Resources/MeshUid.hpp"
#include "Resources/ModelManager.hpp"
#include "Resources/ResourceManagers.hpp"

class RenderObjectSerializer final : public ComponentSerializer
{
private:
	Renderer* renderer;
	kokko::ResourceManagers res;

public:
	RenderObjectSerializer(Renderer* renderer, const kokko::ResourceManagers& resourceManagers) :
		renderer(renderer),
		res(resourceManagers)
	{
	}

	virtual uint32_t GetComponentTypeNameHash() override
	{
		return "render"_hash;
	}

	virtual void DeserializeComponent(const YAML::Node& map, Entity entity) override
	{
		YAML::Node meshNode = map["mesh"];
		YAML::Node materialNode = map["material"];

		if (meshNode.IsDefined() && meshNode.IsScalar() &&
			materialNode.IsDefined() && materialNode.IsScalar())
		{
			RenderObjectId renderObj = renderer->AddRenderObject(entity);

			const std::string& meshUidStr = meshNode.Scalar();
			auto meshUidOpt = kokko::MeshUid::FromString(ArrayView(meshUidStr.c_str(), meshUidStr.length()));

			if (meshUidOpt.HasValue())
			{
				kokko::MeshUid meshUid = meshUidOpt.GetValue();
				kokko::ModelId modelId = res.modelManager->FindModelByUid(meshUid.modelUid);

				if (modelId != kokko::ModelId::Null)
				{
					auto modelMeshes = res.modelManager->GetModelMeshes(modelId);
					
					if (modelMeshes.GetCount() > meshUid.meshIndex)
						renderer->SetMeshId(renderObj, modelMeshes[meshUid.meshIndex].meshId);
				}
			}

			const std::string& matUidStr = materialNode.Scalar();
			auto materialUid = kokko::Uid::FromString(ArrayView(matUidStr.c_str(), matUidStr.length()));

			if (materialUid.HasValue())
			{
				MaterialId materialId = res.materialManager->FindMaterialByUid(materialUid.GetValue());

				// We don't care if the material isn't found
				// Renderer will use fallback material if Material::Null is specified
				RenderOrderData data;
				data.material = materialId;

				if (materialId != MaterialId::Null)
					data.transparency = res.materialManager->GetMaterialTransparency(materialId);
				else
					data.transparency = TransparencyType::Opaque;

				renderer->SetOrderData(renderObj, data);
			}
		}
	}

	virtual void SerializeComponent(YAML::Emitter& out, Entity entity) override
	{
		RenderObjectId renderObj = renderer->Lookup(entity);
		if (renderObj != RenderObjectId::Null)
		{
			MeshId meshId = renderer->GetMeshId(renderObj);

			Optional<kokko::Uid> modelUidOpt;
			if (meshId != MeshId::Null)
				modelUidOpt = res.meshManager->GetUid(meshId);

			char meshUidStr[kokko::MeshUid::StringLength + 1];
			meshUidStr[0] = '\0';

			if (modelUidOpt.HasValue())
			{
				kokko::ModelId modelId = res.modelManager->FindModelByUid(modelUidOpt.GetValue());

				if (modelId != kokko::ModelId::Null)
				{
					auto modelMeshes = res.modelManager->GetModelMeshes(modelId);
					auto modelMesh = modelMeshes.FindIf(
						[meshId](auto& val) { return val.meshId == meshId; });

					if (modelMesh != nullptr)
					{
						kokko::MeshUid meshUid;
						meshUid.modelUid = modelUidOpt.GetValue();
						meshUid.meshIndex = static_cast<uint32_t>(modelMesh - modelMeshes.GetData());

						meshUid.WriteTo(meshUidStr);
						meshUidStr[kokko::MeshUid::StringLength] = '\0';
					}
				}
			}

			char materialUidStr[kokko::Uid::StringLength + 1];
			materialUidStr[0] = '\0';

			MaterialId materialId = renderer->GetOrderData(renderObj).material;
			if (materialId != MaterialId::Null)
			{
				kokko::Uid materialUid = res.materialManager->GetMaterialUid(materialId);
				materialUid.WriteTo(materialUidStr);
				materialUidStr[kokko::Uid::StringLength] = '\0';
			}

			out << YAML::BeginMap;
			out << YAML::Key << GetComponentTypeKey() << YAML::Value << "render";
			out << YAML::Key << "mesh" << YAML::Value << meshUidStr;
			out << YAML::Key << "material" << YAML::Value << materialUidStr;
			out << YAML::EndMap;
		}
	}
};
