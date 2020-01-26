#pragma once

#include "Core/Buffer.hpp"

namespace File
{
	bool ReadBinary(const char* path, Buffer<unsigned char>& output);
	bool ReadText(const char* path, Buffer<char>& output);
	bool Write(const char* path, BufferRef<char> content, bool append);
}
