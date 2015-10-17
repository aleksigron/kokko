#pragma once

#include "Buffer.h"

namespace File
{
	Buffer<unsigned char> Read(const char* path);
}