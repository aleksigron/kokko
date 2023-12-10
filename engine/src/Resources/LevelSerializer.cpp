#include "Resources/LevelSerializer.hpp"

#include "ryml.hpp"

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

namespace
{

void EmitYamlTreeToString(const ryml::Tree& tree, String& out)
{
	// Figure out how long output we need, resize string and then output
	ryml::csubstr output = ryml::emit_yaml(tree, tree.root_id(), ryml::substr{}, false);
	out.Resize(output.len);
	output = ryml::emit_yaml(tree, tree.root_id(), ryml::substr(out.GetData(), out.GetLength()), false);
}

} // namespace

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

void LevelSerializer::DeserializeFromString(MutableStringView data)
{
	KOKKO_PROFILE_FUNCTION();

	// Create scope to mark GPU frame capture
	auto scope = renderDevice->CreateDebugScope(0, kokko::ConstStringView("World_DeserializeLevel"));

	ryml::Tree tree = ryml::parse_in_place(ryml::substr(data.str, data.len));

	auto objects = tree.rootref().find_child("objects");
	if (objects.valid() && objects.is_seq())
	{
		CreateObjects(objects, SceneObjectId::Null);
	}
}

void LevelSerializer::SerializeToString(kokko::String& out)
{
	KOKKO_PROFILE_FUNCTION();

	ryml::Tree tree;
	ryml::NodeRef root = tree.rootref();
	root |= ryml::MAP;

	EntityManager* entityManager = world->GetEntityManager();
	Scene* scene = world->GetScene();

	auto entitySeq = root["objects"];
	entitySeq |= ryml::SEQ;

	for (Entity entity : *entityManager)
	{
		SceneObjectId sceneObj = scene->Lookup(entity);
		if (sceneObj == SceneObjectId::Null || scene->GetParent(sceneObj) == SceneObjectId::Null)
			WriteEntity(entitySeq, entity, sceneObj);
	}

	EmitYamlTreeToString(tree, out);
}

void LevelSerializer::DeserializeEntitiesFromString(ConstStringView data, SceneObjectId parent)
{
	KOKKO_PROFILE_FUNCTION();

	ryml::Tree tree = ryml::parse_in_arena(ryml::csubstr(data.str, data.len));

	CreateObjects(tree.rootref(), parent);
}

void LevelSerializer::SerializeEntitiesToString(ArrayView<Entity> serializeEntities, kokko::String& serializedOut)
{
	KOKKO_PROFILE_FUNCTION();

	Scene* scene = world->GetScene();

	ryml::Tree tree;
	ryml::NodeRef root = tree.rootref();
	root |= ryml::SEQ;
	
	for (Entity entity : serializeEntities)
	{
		SceneObjectId sceneObj = scene->Lookup(entity);
		WriteEntity(root, entity, sceneObj);
	}

	EmitYamlTreeToString(tree, serializedOut);
}

void LevelSerializer::WriteEntity(c4::yml::NodeRef& entitySeq, Entity entity, SceneObjectId sceneObj)
{
	EntityManager* entityManager = world->GetEntityManager();
	Scene* scene = world->GetScene();

	ryml::NodeRef entityNode = entitySeq.append_child();
	entityNode |= ryml::MAP;

	const char* name = entityManager->GetDebugName(entity);
	if (name != nullptr)
		entityNode["entity_name"] = "mesh";

	ryml::NodeRef componentArray = entityNode["components"];
	componentArray |= ryml::SEQ;

	transformSerializer->Serialize(componentArray, entity, sceneObj);

	for (ComponentSerializer* serializer : componentSerializers)
	{
		serializer->SerializeComponent(componentArray, entity);
	}

	SceneObjectId firstChild = scene->GetFirstChild(sceneObj);
	if (firstChild != SceneObjectId::Null)
	{
		ryml::NodeRef childArray = entityNode["children"];
		childArray |= ryml::SEQ;

		SceneObjectId child = firstChild;
		while (child != SceneObjectId::Null)
		{
			Entity childEntity = scene->GetEntity(child);

			WriteEntity(childArray, childEntity, child);

			child = scene->GetNextSibling(child);
		}
	}
}

void LevelSerializer::CreateObjects(const c4::yml::ConstNodeRef& objectSeq, SceneObjectId parent)
{
	for (auto node : objectSeq)
	{
		if (node.is_map())
		{
			EntityManager* entityManager = world->GetEntityManager();
			Entity entity = entityManager->Create();

			SceneObjectId createdTransform = SceneObjectId::Null;

			auto entityNameNode = node.find_child("entity_name");
			if (entityNameNode.valid() && entityNameNode.has_val())
			{
				auto nameStr = entityNameNode.val();
				entityManager->SetDebugName(entity, ConstStringView(nameStr.str, nameStr.len));
			}

			auto componentsNode = node.find_child("components");
			if (componentsNode.valid() && componentsNode.is_seq())
			{
				createdTransform = CreateComponents(componentsNode, entity, parent);
			}

			auto childrenNode = node.find_child("children");
			if (childrenNode.valid() && childrenNode.is_seq())
			{
				if (createdTransform != SceneObjectId::Null)
					CreateObjects(childrenNode, createdTransform);
				else
					KK_LOG_ERROR("LevelSerializer: Children were specified on an object with no transform component, ignoring children");
			}
		}
	}
}

SceneObjectId LevelSerializer::CreateComponents(
	const c4::yml::ConstNodeRef& componentSeq, Entity entity, SceneObjectId parent)
{
	SceneObjectId createdTransform = SceneObjectId::Null;

	for (auto node : componentSeq)
	{
		if (node.is_map())
		{
			auto typeNode = node.find_child(ComponentTypeKey);
			if (typeNode.valid() && typeNode.has_val())
			{
				auto typeStr = typeNode.val();
				uint32_t typeHash = kokko::HashString(typeStr.data(), typeStr.size());

				if (typeHash == "transform"_hash)
					createdTransform = transformSerializer->Deserialize(node, entity, parent);
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
						foundSerializer->DeserializeComponent(node, entity);
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
