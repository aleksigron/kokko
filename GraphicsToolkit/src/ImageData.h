#pragma once

#include <OpenGL/gltypes.h>

#include "Vec2.h"

struct ImageData
{
	Vec2i size;

	unsigned char* data;
	unsigned long dataSize;

	GLenum pixelFormat;

	bool LoadPng(const char* filePath);

	void DeallocateData();
};