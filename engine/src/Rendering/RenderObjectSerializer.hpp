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
	ResourceManagers resourceManagers;

public:
	RenderObjectSerializer(Renderer* renderer, ResourceManagers resourceManagers) :
		renderer(renderer),
		resourceManagers(resourceManagers)
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

			const std::string& meshStr = meshNode.Scalar();
			StringRef meshPath(meshStr.data(), meshStr.size());
			MeshId meshId = resourceManagers.meshManager->GetIdByPath(meshPath);

			assert(meshId != MeshId::Null);

			renderer->SetMeshId(renderObj, meshId);

			const std::string& materialStr = materialNode.Scalar();
			StringRef materialPath(materialStr.data(), materialStr.size());
			MaterialId materialId = resourceManagers.materialManager->GetIdByPath(materialPath);

			assert(materialId != MaterialId::Null);

			RenderOrderData data;
			data.material = materialId;
			data.transparency = resourceManagers.materialManager->GetMaterialData(materialId).transparency;

			renderer->SetOrderData(renderObj, data);
		}
	}

	virtual void SerializeComponent(YAML::Emitter& out, Entity entity) override
	{
		RenderObjectId renderObj = renderer->Lookup(entity);
		if (renderObj != RenderObjectId::Null)
		{
			MeshId meshId = renderer->GetMeshId(renderObj);

			const char* meshPath = nullptr;
			if (meshId != MeshId::Null)
			{
				meshPath = resourceManagers.meshManager->GetPath(meshId);
			}

			MaterialId materialId = renderer->GetOrderData(renderObj).material;
			const char* materialPath = nullptr;
			if (materialId != MaterialId::Null)
			{
				const MaterialData& material = resourceManagers.materialManager->GetMaterialData(materialId);
				materialPath = material.materialPath;
			}

			// We can't reference resources that have been created at runtime
			if (meshPath != nullptr && materialPath != nullptr)
			{
				out << YAML::BeginMap;
				out << YAML::Key << GetComponentTypeKey() << YAML::Value << "render";
				out << YAML::Key << "mesh" << YAML::Value << meshPath;
				out << YAML::Key << "material" << YAML::Value << materialPath;
				out << YAML::EndMap;
			}
		}
	}
};
