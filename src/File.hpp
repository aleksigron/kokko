#pragma once

#include "Buffer.hpp"

namespace File
{
	Buffer<unsigned char> ReadBinary(const char* path);
	Buffer<char> ReadText(const char* path);
}
