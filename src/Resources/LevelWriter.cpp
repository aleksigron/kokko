#include "Resources/LevelWriter.hpp"

#include <fstream>

#include "yaml-cpp/yaml.h"

#include "Engine/ComponentSerializer.hpp"
#include "Engine/EntityManager.hpp"
#include "Engine/World.hpp"

#include "Graphics/EnvironmentManager.hpp"
#include "Graphics/ParticleSystem.hpp"
#include "Graphics/Scene.hpp"
#include "Graphics/TerrainSystem.hpp"

#include "Rendering/CameraSystem.hpp"
#include "Rendering/LightManager.hpp"
#include "Rendering/Renderer.hpp"

#include "Resources/MaterialManager.hpp"
#include "Resources/MeshManager.hpp"
#include "Resources/YamlCustomTypes.hpp"

static const char* const ComponentTypeKey = "component_type";

LevelWriter::LevelWriter(World* world, const ResourceManagers& resManagers, ArrayView<ComponentSerializer*> componentSerializers) :
	world(world),
	resourceManagers(resManagers),
	componentSerializers(componentSerializers)
{
}

bool LevelWriter::WriteToFile(const char* filePath)
{
	EntityManager* entityManager = world->GetEntityManager();
	Scene* scene = world->GetScene();

	std::ofstream outStream(filePath);

	if (outStream.is_open() == false)
		return false;

	YAML::Emitter out(outStream);

	out << YAML::BeginMap;

	int environmentId = scene->GetEnvironmentId();
	if (environmentId >= 0)
	{
		const char* sourcePath = resourceManagers.environmentManager->GetEnvironmentSourcePath(environmentId);
		out << YAML::Key << "environment" << YAML::Value << sourcePath;
	}

	out << YAML::Key << "objects" << YAML::Value << YAML::BeginSeq;
	for (Entity entity : *entityManager)
	{
		SceneObjectId sceneObj = scene->Lookup(entity);
		if (sceneObj == SceneObjectId::Null || scene->GetParent(sceneObj) == SceneObjectId::Null)
			WriteEntity(out, entity, sceneObj);
	}
	out << YAML::EndSeq; // objects

	out << YAML::EndMap;

	return true;
}

void LevelWriter::WriteEntity(YAML::Emitter& out, Entity entity, SceneObjectId sceneObj)
{
	EntityManager* entityManager = world->GetEntityManager();
	Scene* scene = world->GetScene();

	out << YAML::BeginMap;

	const char* name = entityManager->GetDebugName(entity);
	if (name != nullptr)
		out << YAML::Key << "entity_name" << YAML::Value << name;

	out << YAML::Key << "components" << YAML::Value << YAML::BeginSeq;

	WriteTransformComponent(out, entity, sceneObj);

	for (ComponentSerializer* serializer : componentSerializers)
	{
		serializer->SerializeComponent(out, entity);
	}

	out << YAML::EndSeq; // components

	SceneObjectId firstChild = scene->GetFirstChild(sceneObj);
	if (firstChild != SceneObjectId::Null)
	{
		out << YAML::Key << "children" << YAML::Value << YAML::BeginSeq;

		SceneObjectId child = firstChild;
		while (child != SceneObjectId::Null)
		{
			Entity childEntity = scene->GetEntity(child);

			WriteEntity(out, childEntity, child);

			child = scene->GetNextSibling(child);
		}

		out << YAML::EndSeq;
	}

	out << YAML::EndMap;
}

void LevelWriter::WriteTransformComponent(YAML::Emitter& out, Entity entity, SceneObjectId sceneObj)
{
	Scene* scene = world->GetScene();

	if (sceneObj != SceneObjectId::Null)
	{
		const SceneEditTransform& transform = scene->GetEditTransform(sceneObj);

		out << YAML::BeginMap;
		out << YAML::Key << ComponentTypeKey << YAML::Value << "transform";
		out << YAML::Key << "position" << YAML::Value << transform.translation;
		out << YAML::Key << "rotation" << YAML::Value << transform.rotation;
		out << YAML::Key << "scale" << YAML::Value << transform.scale;
		out << YAML::EndMap;
	}
}
