#include "Scripting/ScriptSystem.hpp"

#include "Core/Core.hpp"

#include "Engine/Engine.hpp"

#include "Scripting/NativeScriptComponent.hpp"

namespace kokko
{

ScriptSystem::ScriptSystem(Allocator* allocator) :
	app(nullptr),
	allocator(allocator),
	scripts(allocator),
	scriptsToInit(allocator),
	scriptsToDestroy(allocator),
	entityMap(allocator)
{
}

ScriptSystem::~ScriptSystem()
{
}

void ScriptSystem::AddScriptInternal(Entity entity, NativeScriptComponent* script)
{
	script->entity = entity;

	size_t scriptIndex = scripts.GetCount();
	scripts.PushBack(script);
	scriptsToInit.PushBack(script);

	auto* mapPair = entityMap.Insert(entity.id);
	mapPair->second = static_cast<unsigned int>(scriptIndex);
}

void ScriptSystem::UpdateScripts(kokko::World* world, InputManager* inputManager)
{
	KOKKO_PROFILE_FUNCTION();

	ScriptContext scriptContext;
	scriptContext.app = app;
	scriptContext.world = world;
	scriptContext.inputManager = inputManager;

	for (size_t i = 0, count = scriptsToInit.GetCount(); i < count; ++i)
		scriptsToInit[i]->OnCreate(scriptContext);

	for (size_t i = 0, count = scriptsToDestroy.GetCount(); i < count; ++i)
		scriptsToDestroy[i]->OnDestroy(scriptContext);

	scriptsToInit.Clear();
	scriptsToDestroy.Clear();

	for (size_t i = 0, count = scripts.GetCount(); i < count; ++i)
		scripts[i]->OnUpdate(scriptContext);
}

} // namespace kokko
