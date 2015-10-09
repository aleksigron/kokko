#pragma once

#include "Vec2.h"
#include "Vec3.h"

struct Vertex_PosCol
{
	Vec3f position;
	Vec3f color;
	
	static const int size;
	
	static const int posElements;
	static const unsigned posElemType;
	static const void* posOffset;
	
	static const int colElements;
	static const unsigned colElemType;
	static const void* colOffset;
};

struct Vertex_PosNorCol
{
	Vec3f position;
	Vec3f normal;
	Vec3f color;

	static const int size;

	static const int posElements;
	static const unsigned posElemType;
	static const void* posOffset;

	static const int norElements;
	static const unsigned norElemType;
	static const void* norOffset;

	static const int colElements;
	static const unsigned colElemType;
	static const void* colOffset;
};

struct Vertex_PosTex
{
	Vec3f position;
	Vec2f texCoord;

	static const int size;

	static const int posElements;
	static const unsigned posElemType;
	static const void* posOffset;

	static const int texCoordElements;
	static const unsigned texCoordElemType;
	static const void* texCoordOffset;
};