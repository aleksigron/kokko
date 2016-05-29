#pragma once

#include "Vec2.hpp"
#include "Vec3.hpp"

struct Vertex3f2f
{
	Vec3f a;
	Vec2f b;

	static const int size;

	static const int aElemCount;
	static const unsigned aElemType;
	static const void* aOffset;

	static const int bElemCount;
	static const unsigned bElemType;
	static const void* bOffset;
};

struct Vertex3f3f
{
	Vec3f a;
	Vec3f b;

	static const int size;

	static const int aElemCount;
	static const unsigned aElemType;
	static const void* aOffset;

	static const int bElemCount;
	static const unsigned bElemType;
	static const void* bOffset;
};

struct Vertex3f3f2f
{
	Vec3f a;
	Vec3f b;
	Vec2f c;

	static const int size;

	static const int aElemCount;
	static const unsigned aElemType;
	static const void* aOffset;

	static const int bElemCount;
	static const unsigned bElemType;
	static const void* bOffset;

	static const int cElemCount;
	static const unsigned cElemType;
	static const void* cOffset;
};

struct Vertex3f3f3f
{
	Vec3f a;
	Vec3f b;
	Vec2f c;

	static const int size;

	static const int aElemCount;
	static const unsigned aElemType;
	static const void* aOffset;

	static const int bElemCount;
	static const unsigned bElemType;
	static const void* bOffset;

	static const int cElemCount;
	static const unsigned cElemType;
	static const void* cOffset;
};

struct Vertex_PosNor
{
	Vec3f position;
	Vec3f normal;
	
	static const int size;
	
	static const int posElements;
	static const unsigned posElemType;
	static const void* posOffset;
	
	static const int norElements;
	static const unsigned norElemType;
	static const void* norOffset;
};

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

struct Vertex_PosNorTex
{
	Vec3f position;
	Vec3f normal;
	Vec2f texCoord;

	static const int size;

	static const int posElements;
	static const unsigned posElemType;
	static const void* posOffset;

	static const int norElements;
	static const unsigned norElemType;
	static const void* norOffset;

	static const int texCoordElements;
	static const unsigned texCoordElemType;
	static const void* texCoordOffset;
};
