#pragma once

#include <filesystem>

#include "Core/Hash.hpp"

namespace kokko
{
inline uint32_t Hash32(const std::filesystem::path& value, uint32_t seed)
{
	const auto& native = value.native();
	return Hash32(native.c_str(), native.length() * sizeof(std::filesystem::path::value_type), seed);
}
}
