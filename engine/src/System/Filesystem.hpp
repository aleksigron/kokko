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
	virtual bool Write(const char* path, ArrayView<const uint8_t> content, bool append) = 0;
};
