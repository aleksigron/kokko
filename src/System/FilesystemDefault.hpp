#pragma once

class Allocator;

#include <cstdint>

#include "Core/Array.hpp"

#include "System/Filesystem.hpp"

class FilesystemDefault : public Filesystem
{
public:
	virtual bool ReadBinary(const char* path, Array<unsigned char>& output) override;

	virtual bool ReadText(const char* path, Array<char>& output) override;
	virtual bool ReadText(const char* path, Allocator* allocator, char*& strOut, size_t& lenOut) override;

	virtual bool Write(const char* path, ArrayView<char> content, bool append) override;
};
