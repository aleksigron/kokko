#pragma once

#include "Core/Buffer.hpp"
#include "Core/StringRef.hpp"

namespace File
{
	bool ReadBinary(StringRef path, Buffer<unsigned char>& output);
	bool ReadBinary(const char* path, Buffer<unsigned char>& output);

	bool ReadText(StringRef path, Buffer<char>& output);
	bool ReadText(const char* path, Buffer<char>& output);

	bool Write(StringRef path, BufferRef<char> content, bool append);
	bool Write(const char* path, BufferRef<char> content, bool append);
}
