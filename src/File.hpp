#pragma once

#include "Core/Buffer.hpp"
#include "Core/StringRef.hpp"

namespace File
{
	Buffer<unsigned char> ReadBinary(StringRef path);
	Buffer<unsigned char> ReadBinary(const char* path);

	Buffer<char> ReadText(StringRef path);
	Buffer<char> ReadText(const char* path);

	bool Write(StringRef path, BufferRef<char> content, bool append);
	bool Write(const char* path, BufferRef<char> content, bool append);
}
