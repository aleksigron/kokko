#pragma once

#include <cstdint>

namespace kokko
{

struct Entity
{
	uint32_t id;

	Entity() = default;
	explicit Entity(uint32_t id) : id(id) {}
	bool operator==(Entity other) { return id == other.id; }
	bool operator!=(Entity other) { return !operator==(other); }

	static const Entity Null;
};

} // namespace kokko
