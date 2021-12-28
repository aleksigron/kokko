#pragma once

#include <cstddef>
#include <cstdint>

#include "Core/Array.hpp"

class Allocator;

namespace kokko
{
class String;
}

class Filesystem
{
public:
	virtual ~Filesystem() {}

	virtual bool ReadBinary(const char* path, Array<uint8_t>& output) = 0;
	virtual bool ReadText(const char* path, kokko::String& output) = 0;
	virtual bool WriteText(const char* path, ArrayView<const char> content, bool append) = 0;
};
