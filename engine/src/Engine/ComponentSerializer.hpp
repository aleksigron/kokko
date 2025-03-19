#pragma once

#include <cstdint>

namespace c4
{
namespace yml
{
class ConstNodeRef;
class NodeRef;
}
}

namespace kokko
{

struct Entity;

class ComponentSerializer
{
public:
	virtual ~ComponentSerializer() {};

	static constexpr const char* GetComponentTypeKey() { return "component_type"; }

	virtual uint32_t GetComponentTypeNameHash() = 0;
	virtual void DeserializeComponent(const c4::yml::ConstNodeRef& map, Entity entity) = 0;
	virtual void SerializeComponent(c4::yml::NodeRef& componentArray, Entity entity) = 0;
};

} // namespace kokko