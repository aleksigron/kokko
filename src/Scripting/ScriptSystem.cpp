#include "Scripting/ScriptSystem.hpp"

#include "Core/Core.hpp"

#include "Engine/Engine.hpp"

#include "Scripting/NativeScriptComponent.hpp"

#include "System/Window.hpp"

ScriptSystem::ScriptSystem(Engine* engine, Allocator* allocator) :
	engine(engine),
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

	unsigned int scriptIndex = scripts.GetCount();
	scripts.PushBack(script);
	scriptsToInit.PushBack(script);

	HashMap<unsigned int, unsigned int>::KeyValuePair* mapPair = entityMap.Insert(entity.id);
	mapPair->second = scriptIndex;
}

void ScriptSystem::UpdateScripts()
{
	KOKKO_PROFILE_FUNCTION();

	ScriptContext scriptContext;
	scriptContext.app = app;
	scriptContext.inputManager = engine->GetMainWindow()->GetInputManager();
	scriptContext.sceneManager = engine->GetSceneManager();

	for (unsigned int i = 0, count = scriptsToInit.GetCount(); i < count; ++i)
		scriptsToInit[i]->OnCreate(scriptContext);

	for (unsigned int i = 0, count = scriptsToDestroy.GetCount(); i < count; ++i)
		scriptsToDestroy[i]->OnDestroy(scriptContext);

	scriptsToInit.Clear();
	scriptsToDestroy.Clear();

	for (unsigned int i = 0, count = scripts.GetCount(); i < count; ++i)
		scripts[i]->OnUpdate(scriptContext);
}
