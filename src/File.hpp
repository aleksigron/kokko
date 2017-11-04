#pragma once

#include "Buffer.hpp"
#include "StringRef.hpp"

namespace File
{
	Buffer<unsigned char> ReadBinary(StringRef path);
	Buffer<unsigned char> ReadBinary(const char* path);

	Buffer<char> ReadText(StringRef path);
	Buffer<char> ReadText(const char* path);
}
