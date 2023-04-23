#include "Resources/LevelSerializer.hpp"

#include <sstream>

#include "Engine/ComponentSerializer.hpp"
#include "Engine/EntityManager.hpp"
#include "Engine/World.hpp"

#include "Graphics/ParticleEmitterSerializer.hpp"
#include "Graphics/TerrainSerializer.hpp"
#include "Graphics/TransformSerializer.hpp"

#include "Rendering/CameraSerializer.hpp"
#include "Rendering/LightSerializer.hpp"
#include "Rendering/MeshComponentSerializer.hpp"
#include "Rendering/RenderDevice.hpp"

#include "Graphics/EnvironmentSerializer.hpp"
#include "Graphics/Scene.hpp"

#include "Resources/YamlCustomTypes.hpp"

static const char* const ComponentTypeKey = "component_type";

namespace kokko
{

LevelSerializer::LevelSerializer(Allocator* allocator, kokko::render::Device* renderDevice) :
	allocator(allocator),
	renderDevice(renderDevice),
	world(nullptr),
	resourceManagers(kokko::ResourceManagers{}),
	transformSerializer(nullptr),
	componentSerializers(allocator)
{
}

LevelSerializer::~LevelSerializer()
{
	for (ComponentSerializer* serializer : componentSerializers)
		allocator->MakeDelete(serializer);

	allocator->MakeDelete(transformSerializer);
}

void LevelSerializer::Initialize(kokko::World* world, const kokko::ResourceManagers& resourceManagers)
{
	this->world = world;
	this->resourceManagers = resourceManagers;

	transformSerializer = allocator->MakeNew<TransformSerializer>(world->GetScene());

	componentSerializers.PushBack(allocator->MakeNew<MeshComponentSerializer>(world->GetMeshComponentSystem(), resourceManagers));
	componentSerializers.PushBack(allocator->MakeNew<LightSerializer>(world->GetLightManager()));
	componentSerializers.PushBack(allocator->MakeNew<CameraSerializer>(world->GetCameraSystem()));
	componentSerializers.PushBack(allocator->MakeNew<ParticleEmitterSerializer>(world->GetParticleSystem()));
	componentSerializers.PushBack(allocator->MakeNew<TerrainSerializer>(
		world->GetTerrainSystem(), resourceManagers.textureManager));
	componentSerializers.PushBack(allocator->MakeNew<kokko::EnvironmentSerializer>(world->GetEnvironmentSystem()));
}

void LevelSerializer::DeserializeFromString(const char* data)
{
	KOKKO_PROFILE_FUNCTION();

	// Create scope to mark GPU frame capture
	auto scope = renderDevice->CreateDebugScope(0, kokko::ConstStringView("World_DeserializeLevel"));

	YAML::Node node = YAML::Load(data);

	const YAML::Node objects = node["objects"];
	if (objects.IsDefined() && objects.IsSequence())
	{
		CreateObjects(objects, SceneObjectId::Null);
	}
}

void LevelSerializer::SerializeToString(kokko::String& out)
{
	KOKKO_PROFILE_FUNCTION();

	std::stringstream outStream;
	YAML::Emitter emitter(outStream);

	EntityManager* entityManager = world->GetEntityManager();
	Scene* scene = world->GetScene();

	emitter << YAML::BeginMap;

	emitter << YAML::Key << "objects" << YAML::Value << YAML::BeginSeq;

	for (Entity entity : *entityManager)
	{
		SceneObjectId sceneObj = scene->Lookup(entity);
		if (sceneObj == SceneObjectId::Null || scene->GetParent(sceneObj) == SceneObjectId::Null)
			WriteEntity(emitter, entity, sceneObj);
	}
	emitter << YAML::EndSeq; // objects

	emitter << YAML::EndMap;

	std::string str = outStream.str();
	out.Assign(kokko::ConstStringView(str.c_str(), str.length()));
}

void LevelSerializer::DeserializeEntitiesFromString(const char* data, SceneObjectId parent)
{
	KOKKO_PROFILE_FUNCTION();

	YAML::Node node = YAML::Load(data);

	CreateObjects(node, parent);
}

void LevelSerializer::SerializeEntitiesToString(ArrayView<Entity> serializeEntities, kokko::String& serializedOut)
{
	KOKKO_PROFILE_FUNCTION();

	Scene* scene = world->GetScene();

	YAML::Emitter emitter;

	emitter << YAML::BeginSeq;
	for (Entity entity : serializeEntities)
	{
		SceneObjectId sceneObj = scene->Lookup(entity);
		WriteEntity(emitter, entity, sceneObj);
	}
	emitter << YAML::EndSeq;

	serializedOut.Assign(kokko::ConstStringView(emitter.c_str(), emitter.size()));
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

	transformSerializer->Serialize(out, entity, sceneObj);

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
					KK_LOG_ERROR("LevelSerializer: Children were specified on an object with no transform component, ignoring children");
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
				uint32_t typeHash = kokko::HashString(typeStr.data(), typeStr.size());

				if (typeHash == "transform"_hash)
					createdTransform = transformSerializer->Deserialize(*itr, entity, parent);
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
						KK_LOG_ERROR("LevelSerializer: Unknown component type");
				}
			}
			else
				KK_LOG_ERROR("LevelSerializer: Invalid component type");
		}
	}

	return createdTransform;
}

} // namespace kokko
