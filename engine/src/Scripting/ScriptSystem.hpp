#pragma once

#include <type_traits>

#include "Core/Array.hpp"
#include "Core/HashMap.hpp"

#include "Engine/Entity.hpp"

namespace kokko
{
class World;
}

class InputManager;
class Allocator;
class NativeScriptComponent;

class ScriptSystem
{
private:
	void* app;
	Allocator* allocator;

	kokko::Array<NativeScriptComponent*> scripts;
	kokko::Array<NativeScriptComponent*> scriptsToInit;
	kokko::Array<NativeScriptComponent*> scriptsToDestroy;

	HashMap<unsigned int, unsigned int> entityMap;

	void AddScriptInternal(Entity entity, NativeScriptComponent* script);

public:
	explicit ScriptSystem(Allocator* allocator);
	ScriptSystem(const ScriptSystem&) = delete;
	ScriptSystem(ScriptSystem&&) = delete;
	~ScriptSystem();

	ScriptSystem& operator=(const ScriptSystem&) = delete;
	ScriptSystem& operator=(ScriptSystem&&) = delete;

	void SetAppPointer(void* app) { this->app = app; }

	template <typename ScriptType>
	ScriptType* AddScript(Entity entity)
	{
		static_assert(std::is_base_of<NativeScriptComponent, ScriptType>::value, "Script must inherit NativeScriptComponent");

		ScriptType* scriptInstance = allocator->MakeNew<ScriptType>();

		AddScriptInternal(entity, scriptInstance);

		return scriptInstance;
	}

	template <typename ScriptType>
	ScriptType* GetEntityScriptAs(Entity e)
	{
		auto* pair = entityMap.Lookup(e.id);
		
		if (pair != nullptr)
			return dynamic_cast<ScriptType*>(scripts[pair->second]);
		else
			return nullptr;
	}

	void UpdateScripts(kokko::World* world, InputManager* inputManager);
};
