#include "Resources/LevelWriter.hpp"

#include <fstream>

#include "yaml-cpp/yaml.h"

#include "Engine/Engine.hpp"

#include "Entity/EntityManager.hpp"

#include "Graphics/World.hpp"

#include "Rendering/Renderer.hpp"

#include "Resources/MaterialManager.hpp"
#include "Resources/MeshManager.hpp"
#include "Resources/YamlCustomTypes.hpp"

LevelWriter::LevelWriter(Engine* engine) :
	world(engine->GetWorld()),
	renderer(engine->GetRenderer()),
	meshManager(engine->GetMeshManager()),
	materialManager(engine->GetMaterialManager()),
	entityManager(engine->GetEntityManager()),
	lightManager(engine->GetLightManager()),
	environmentManager(engine->GetEnvironmentManager())
{
}

bool LevelWriter::WriteToFile(const char* filePath)
{
	std::ofstream outStream(filePath);

	if (outStream.is_open() == false)
		return false;

	YAML::Emitter out(outStream);
	out << YAML::BeginMap;
	out << YAML::Key << "objects" << YAML::Value << YAML::BeginSeq;
	for (Entity entity : *entityManager)
	{
		SceneObjectId sceneObj = world->Lookup(entity);
		if (sceneObj != SceneObjectId::Null && world->GetParent(sceneObj) == SceneObjectId::Null)
			SerializeEntity(out, entity, sceneObj);
	}

	out << YAML::EndSeq; // objects

	out << YAML::EndMap;

	return true;
}

void LevelWriter::SerializeEntity(YAML::Emitter& out, Entity entity, SceneObjectId sceneObj)
{
	out << YAML::BeginMap;

	const char* name = entityManager->GetDebugName(entity);
	if (name != nullptr)
		out << YAML::Key << "debug_name" << YAML::Value << name;

	const SceneEditTransform& transform = world->GetEditTransform(sceneObj);
	out << YAML::Key << "transform_component" << YAML::Value << YAML::BeginMap;
	out << YAML::Key << "position" << YAML::Value << transform.translation;
	out << YAML::Key << "rotation" << YAML::Value << transform.rotation;
	out << YAML::Key << "scale" << YAML::Value << transform.scale;
	out << YAML::EndMap; // transform_component

	RenderObjectId renderObj = renderer->Lookup(entity);
	if (renderObj != RenderObjectId::Null)
	{
		MeshId meshId = renderer->GetMeshId(renderObj);
		const char* meshPath = meshManager->GetPath(meshId);

		MaterialId materialId = renderer->GetOrderData(renderObj).material;
		const MaterialData& material = materialManager->GetMaterialData(materialId);
		const char* materialPath = material.materialPath;

		// We can't reference resources that have been created at runtime
		if (meshPath != nullptr && materialPath != nullptr)
		{
			out << YAML::Key << "render_component" << YAML::Value << YAML::BeginMap;
			out << YAML::Key << "mesh" << YAML::Value << meshPath;
			out << YAML::Key << "material" << YAML::Value << materialPath;
			out << YAML::EndMap; // render_component
		}
	}

	SceneObjectId firstChild = world->GetFirstChild(sceneObj);
	if (firstChild != SceneObjectId::Null)
	{
		SceneObjectId child = firstChild;

		out << YAML::Key << "children" << YAML::Value << YAML::BeginSeq;

		while (child != SceneObjectId::Null)
		{
			Entity childEntity = world->GetEntity(child);

			SerializeEntity(out, childEntity, child);

			child = world->GetNextSibling(child);
		}

		out << YAML::EndSeq;
	}

	out << YAML::EndMap;
}
