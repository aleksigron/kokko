#pragma once

#include "Vec2.h"

struct ImageData
{
	Vec2i size;

	unsigned char* data;
	unsigned long dataSize;

	bool hasAlpha;

	bool LoadPng(const char* filePath);
	
	void LoadTestData();

	void DeallocateData();
};