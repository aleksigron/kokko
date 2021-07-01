#pragma once

#include "Core/Array.hpp"

namespace File
{
	bool ReadBinary(const char* path, Array<unsigned char>& output);

	bool ReadText(const char* path, Array<char>& output);
	bool ReadText(const char* path, Allocator* allocator, char*& strOut, size_t& lenOut);

	bool Write(const char* path, ArrayView<char> content, bool append);
}
