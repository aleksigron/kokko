#pragma once

#include "Vec2.hpp"
#include "Vec3.hpp"
#include "Vec4.hpp"

struct Vertex3f
{
	Vec3f a;

	static const int size;

	static const int aElemCount;
	static const unsigned aElemType;
	static const void* aOffset;
};

struct Vertex4f
{
	Vec4f a;

	static const int size;

	static const int aElemCount;
	static const unsigned aElemType;
	static const void* aOffset;
};

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
