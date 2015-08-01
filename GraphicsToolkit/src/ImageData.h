#pragma once

#include "Vec2.h"

struct ImageData
{
	Vec2i size;

	unsigned char* data;
	unsigned long dataSize;

	bool hasAlpha;

	void LoadTga(const char* filePath);
	void LoadTestData();
};