#pragma once

#include <cstddef>
#include <cstdint>

#include "Core/Array.hpp"

class Allocator;
class String;

class Filesystem
{
public:
	virtual ~Filesystem() {}

	virtual bool ReadBinary(const char* path, Array<uint8_t>& output) = 0;
	virtual bool ReadText(const char* path, String& output) = 0;
	virtual bool Write(const char* path, ArrayView<char> content, bool append) = 0;
};
