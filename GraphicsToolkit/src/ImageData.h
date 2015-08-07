#pragma once

#include "Vec2.h"

struct ImageData
{
	Vec2i size;

	unsigned char* data;
	unsigned long dataSize;

	unsigned int pixelFormat;

	bool LoadPng(const char* filePath);

	void DeallocateData();
};