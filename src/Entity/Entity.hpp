#pragma once

#include <cstdint>

struct Entity
{
	uint32_t id;

	Entity() = default;
	explicit Entity(uint32_t id) : id(id) {}
	bool operator==(Entity& other) { id = other.id; }
	bool operator!=(Entity& other) { id != other.id; }

	static Entity Null;
};
