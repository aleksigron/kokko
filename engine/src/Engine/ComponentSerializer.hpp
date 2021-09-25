#pragma once

#include <cstdint>

struct Entity;

namespace YAML
{
	class Node;
	class Emitter;
}

class ComponentSerializer
{
public:
	virtual ~ComponentSerializer() {};

	static constexpr const char* GetComponentTypeKey() { return "component_type"; }

	virtual uint32_t GetComponentTypeNameHash() = 0;
	virtual void DeserializeComponent(const YAML::Node& map, Entity entity) = 0;
	virtual void SerializeComponent(YAML::Emitter& out, Entity entity) = 0;
};
