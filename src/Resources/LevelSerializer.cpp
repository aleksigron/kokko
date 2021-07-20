#include "Resources/LevelSerializer.hpp"

#include "Engine/World.hpp"

#include "Graphics/ParticleEmitterSerializer.hpp"
#include "Graphics/TerrainSerializer.hpp"

#include "Rendering/CameraSerializer.hpp"
#include "Rendering/LightSerializer.hpp"
#include "Rendering/RenderObjectSerializer.hpp"

#include "Resources/LevelLoader.hpp"
#include "Resources/LevelWriter.hpp"

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
	LevelLoader loader(world, resourceManagers, componentSerializers.GetView());
	loader.Load(data);
}

bool LevelSerializer::SerializeToFile(const char* filePath)
{
	LevelWriter writer(world, resourceManagers, componentSerializers.GetView());

	return writer.WriteToFile(filePath);
}
