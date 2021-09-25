#pragma once

class Allocator;

#include <cstddef>
#include <cstdint>

#include "Core/Array.hpp"

class Filesystem
{
public:
	virtual ~Filesystem() {}

	virtual bool ReadBinary(const char* path, Array<uint8_t>& output) = 0;

	virtual bool ReadText(const char* path, Array<char>& output) = 0;
	virtual bool ReadText(const char* path, Allocator* allocator, char*& strOut, size_t& lenOut) = 0;

	virtual bool Write(const char* path, ArrayView<char> content, bool append) = 0;
};
