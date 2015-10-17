#pragma once

#include "Buffer.hpp"

namespace File
{
	Buffer<unsigned char> Read(const char* path);
}