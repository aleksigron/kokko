#pragma once

#include "Engine/Entity.hpp"

#include "Scripting/ScriptContext.hpp"

namespace kokko
{

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

} // namespace kokko
