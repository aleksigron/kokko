#include "Resources/LevelLoader.hpp"

#include "yaml-cpp/yaml.h"

#include "Core/Core.hpp"

#include "Engine/ComponentSerializer.hpp"
#include "Engine/EntityManager.hpp"
#include "Engine/World.hpp"

#include "Graphics/EnvironmentManager.hpp"
#include "Graphics/ParticleSystem.hpp"
#include "Graphics/Scene.hpp"
#include "Graphics/TerrainSystem.hpp"

#include "Rendering/CameraSystem.hpp"
#include "Rendering/Renderer.hpp"
#include "Rendering/LightManager.hpp"

#include "Resources/MeshManager.hpp"
#include "Resources/MaterialManager.hpp"
#include "Resources/ValueSerialization.hpp"
#include "Resources/YamlCustomTypes.hpp"

LevelLoader::LevelLoader(
	World* world,
	const ResourceManagers& resourceManagers,
	ArrayView<ComponentSerializer*> componentSerializers):
	world(world),
	resourceManagers(resourceManagers),
	componentSerializers(componentSerializers)
{
}

void LevelLoader::Load(const char* data)
{
	KOKKO_PROFILE_FUNCTION();

	Scene* scene = world->GetScene();

	YAML::Node node = YAML::Load(data);

	const YAML::Node environment = node["environment"];
	if (environment.IsDefined() && environment.IsScalar())
	{
		int envId = resourceManagers.environmentManager->LoadHdrEnvironmentMap(environment.Scalar().c_str());

		assert(envId >= 0);

		scene->SetEnvironmentId(envId);
	}
	else
		scene->SetEnvironmentId(-1);

	const YAML::Node objects = node["objects"];
	if (objects.IsDefined() && objects.IsSequence())
	{
		CreateObjects(objects, SceneObjectId::Null);
	}
}

void LevelLoader::CreateObjects(const YAML::Node& childSequence, SceneObjectId parent)
{
	YAML::const_iterator itr = childSequence.begin();
	YAML::const_iterator end = childSequence.end();

	for (; itr != end; ++itr)
	{
		if (itr->IsMap())
		{
			EntityManager* entityManager = world->GetEntityManager();
			Entity entity = entityManager->Create();

			SceneObjectId createdTransform = SceneObjectId::Null;

			const YAML::Node entityNameNode = (*itr)["entity_name"];
			if (entityNameNode.IsDefined() && entityNameNode.IsScalar())
			{
				const std::string& nameStr = entityNameNode.Scalar();
				entityManager->SetDebugName(entity, nameStr.c_str());
			}

			const YAML::Node componentsNode = (*itr)["components"];
			if (componentsNode.IsDefined() && componentsNode.IsSequence())
			{
				createdTransform = CreateComponents(componentsNode, entity, parent);
			}

			const YAML::Node childrenNode = (*itr)["children"];
			if (childrenNode.IsDefined() && childrenNode.IsSequence())
			{
				if (createdTransform != SceneObjectId::Null)
					CreateObjects(childrenNode, createdTransform);
				else
					KK_LOG_ERROR("LevelLoader: Children were specified on an object with no transform component, ignoring children");
			}
		}
	}
}

SceneObjectId LevelLoader::CreateComponents(const YAML::Node& componentSequence, Entity entity, SceneObjectId parent)
{
	YAML::const_iterator itr = componentSequence.begin();
	YAML::const_iterator end = componentSequence.end();

	SceneObjectId createdTransform = SceneObjectId::Null;

	for (; itr != end; ++itr)
	{
		if (itr->IsMap())
		{
			YAML::Node typeNode = (*itr)["component_type"];
			if (typeNode.IsDefined() && typeNode.IsScalar())
			{
				const std::string& typeStr = typeNode.Scalar();
				uint32_t typeHash = Hash::FNV1a_32(typeStr.data(), typeStr.size());

				if (typeHash == "transform"_hash)
					createdTransform = CreateTransformComponent(*itr, entity, parent);
				else
				{
					ComponentSerializer* foundSerializer = nullptr;

					for (ComponentSerializer* serializer : componentSerializers)
					{
						if (typeHash == serializer->GetComponentTypeNameHash())
						{
							foundSerializer = serializer;
							break;
						}
					}

					if (foundSerializer != nullptr)
						foundSerializer->DeserializeComponent(*itr, entity);
					else
						KK_LOG_ERROR("LevelLoader: Unknown component type");
				}
			}
			else
				KK_LOG_ERROR("LevelLoader: Invalid component type");
		}
	}

	return createdTransform;
}

SceneObjectId LevelLoader::CreateTransformComponent(const YAML::Node& map, Entity entity, SceneObjectId parent)
{
	Scene* scene = world->GetScene();
	SceneObjectId sceneObject = scene->AddSceneObject(entity);

	if (parent != SceneObjectId::Null)
		scene->SetParent(sceneObject, parent);

	SceneEditTransform transform;

	YAML::Node positionNode = map["position"];
	if (positionNode.IsDefined() && positionNode.IsSequence())
		transform.translation = positionNode.as<Vec3f>();

	YAML::Node rotationNode = map["rotation"];
	if (rotationNode.IsDefined() && rotationNode.IsSequence())
		transform.rotation = rotationNode.as<Vec3f>();

	YAML::Node scaleNode = map["scale"];
	if (scaleNode.IsDefined() && scaleNode.IsSequence())
		transform.scale = scaleNode.as<Vec3f>();

	scene->SetEditTransform(sceneObject, transform);

	return sceneObject;
}
