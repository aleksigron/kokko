#pragma once

#include <cassert>

#include "yaml-cpp/yaml.h"

#include "Core/Hash.hpp"

#include "Engine/ComponentSerializer.hpp"
#include "Engine/Entity.hpp"

#include "Rendering/Renderer.hpp"

#include "Resources/MaterialManager.hpp"
#include "Resources/MeshManager.hpp"
#include "Resources/ResourceManagers.hpp"

class RenderObjectSerializer final : public ComponentSerializer
{
private:
	Renderer* renderer;
	ResourceManagers res;

public:
	RenderObjectSerializer(Renderer* renderer, ResourceManagers resourceManagers) :
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
			auto meshUid = kokko::Uid::FromString(ArrayView(meshUidStr.c_str(), meshUidStr.length()));

			if (meshUid.HasValue())
			{
				MeshId meshId = res.meshManager->FindModelByUid(meshUid.GetValue());

				renderer->SetMeshId(renderObj, meshId);
			}

			const std::string& matUidStr = materialNode.Scalar();
			auto materialUid = kokko::Uid::FromString(ArrayView(matUidStr.c_str(), matUidStr.length()));

			if (materialUid.HasValue())
			{
				MaterialId materialId = res.materialManager->FindMaterialByUid(materialUid.GetValue());

				RenderOrderData data;
				data.material = materialId;
				data.transparency = res.materialManager->GetMaterialTransparency(materialId);

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

			kokko::Uid meshUid;
			if (meshId != MeshId::Null)
				meshUid = res.meshManager->GetUid(meshId);

			char meshUidStr[kokko::Uid::StringLength + 1];
			meshUid.WriteTo(meshUidStr);
			meshUidStr[kokko::Uid::StringLength] = '\0';

			MaterialId materialId = renderer->GetOrderData(renderObj).material;
			kokko::Uid materialUid;
			if (materialId != MaterialId::Null)
				materialUid = res.materialManager->GetMaterialUid(materialId);

			char materialUidStr[kokko::Uid::StringLength + 1];
			materialUid.WriteTo(materialUidStr);
			materialUidStr[kokko::Uid::StringLength] = '\0';

			out << YAML::BeginMap;
			out << YAML::Key << GetComponentTypeKey() << YAML::Value << "render";
			out << YAML::Key << "mesh" << YAML::Value << meshUidStr;
			out << YAML::Key << "material" << YAML::Value << materialUidStr;
			out << YAML::EndMap;
		}
	}
};
