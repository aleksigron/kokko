#pragma once

#include "Rectangle.hpp"
#include "Vec2.hpp"

#include "StringRef.hpp"
#include "Buffer.hpp"

struct BitmapGlyph
{
	unsigned int codePoint;
	Vec2f size;
	Vec2f texturePosition;
};

class BitmapFont
{
private:
	unsigned int textureId;
	Vec2f textureSize;

	unsigned int* glyphSkipList;
	static const unsigned int glyphSkipListStep = 16;

	BitmapGlyph* glyphs;
	unsigned int glyphCount;

	int lineHeight;

	static void ParseBitmapRow(StringRef line, unsigned int pixels, unsigned char* bitmapOut);
	static Vec2f CalculateTextureSize(int glyphCount, Vec2f glyphSize);

public:
	BitmapFont();
	~BitmapFont();

	const BitmapGlyph* GetGlyph(unsigned int codePoint) const;

	int GetLineHeight() const { return lineHeight; }

	unsigned int GetTextureDriverId() const { return textureId; }
	Vec2f GetTextureSize() const { return textureSize; }

	bool LoadFromBDF(const Buffer<char>& content);
};
