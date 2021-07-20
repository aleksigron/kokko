#include "Resources/LevelSerializer.hpp"

#include <fstream>

#include "Engine/ComponentSerializer.hpp"
#include "Engine/EntityManager.hpp"
#include "Engine/World.hpp"

#include "Graphics/ParticleEmitterSerializer.hpp"
#include "Graphics/TerrainSerializer.hpp"

#include "Rendering/CameraSerializer.hpp"
#include "Rendering/LightSerializer.hpp"
#include "Rendering/RenderObjectSerializer.hpp"

#include "Graphics/EnvironmentManager.hpp"
#include "Graphics/Scene.hpp"

#include "Resources/YamlCustomTypes.hpp"

static const char* const ComponentTypeKey = "component_type";

LevelSerializer::LevelSerializer(Allocator* allocator) :
	allocator(allocator),
	world(nullptr),
	resourceManagers(ResourceManagers{}),
	componentSerializers(allocator)
{
}

LevelSerializer::~LevelSerializer()
{
	for (ComponentSerializer* serializer : componentSerializers)
		allocator->MakeDelete(serializer);
}

void LevelSerializer::Initialize(World* world, const ResourceManagers& resourceManagers)
{
	this->world = world;
	this->resourceManagers = resourceManagers;

	componentSerializers.PushBack(allocator->MakeNew<RenderObjectSerializer>(world->GetRenderer(), resourceManagers));
	componentSerializers.PushBack(allocator->MakeNew<LightSerializer>(world->GetLightManager()));
	componentSerializers.PushBack(allocator->MakeNew<CameraSerializer>(world->GetCameraSystem()));
	componentSerializers.PushBack(allocator->MakeNew<ParticleEmitterSerializer>(world->GetParticleSystem()));
	componentSerializers.PushBack(allocator->MakeNew<TerrainSerializer>(world->GetTerrainSystem()));
}

void LevelSerializer::DeserializeFromString(const char* data)
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

bool LevelSerializer::SerializeToFile(const char* filePath)
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

void LevelSerializer::WriteEntity(YAML::Emitter& out, Entity entity, SceneObjectId sceneObj)
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

void LevelSerializer::WriteTransformComponent(YAML::Emitter& out, Entity entity, SceneObjectId sceneObj)
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


void LevelSerializer::CreateObjects(const YAML::Node& childSequence, SceneObjectId parent)
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

SceneObjectId LevelSerializer::CreateComponents(const YAML::Node& componentSequence, Entity entity, SceneObjectId parent)
{
	YAML::const_iterator itr = componentSequence.begin();
	YAML::const_iterator end = componentSequence.end();

	SceneObjectId createdTransform = SceneObjectId::Null;

	for (; itr != end; ++itr)
	{
		if (itr->IsMap())
		{
			YAML::Node typeNode = (*itr)[ComponentTypeKey];
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

SceneObjectId LevelSerializer::CreateTransformComponent(const YAML::Node& map, Entity entity, SceneObjectId parent)
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
