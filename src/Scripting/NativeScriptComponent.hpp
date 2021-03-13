#pragma once

#include "Entity/Entity.hpp"

#include "Scripting/ScriptContext.hpp"

class NativeScriptComponent
{
protected:
	Entity entity;

public:
	virtual void OnCreate(const ScriptContext& context) {}
	virtual void OnUpdate(const ScriptContext& context) {}
	virtual void OnDestroy(const ScriptContext& context) {}

	friend class ScriptSystem;
};
