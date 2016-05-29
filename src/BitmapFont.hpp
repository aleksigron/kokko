#pragma once

#include "Rectangle.hpp"
#include "Vec2.hpp"

#include "StringRef.hpp"
#include "Buffer.hpp"

struct BitmapGlyph
{
	unsigned int codePoint;
	Vec2f size;
	Vec2f advance;
	Vec2f drawOffset;
	Vec2f texturePosition;
};

class BitmapFont
{
private:
	unsigned int textureId;
	Vec2f textureSize;

	BitmapGlyph* glyphs;
	unsigned int glyphCount;

	static void ParseBitmapRow(StringRef line, unsigned int pixels, unsigned char* bitmapOut);
	static Vec2f CalculateTextureSize(int glyphCount, Vec2f glyphSize);

public:
	BitmapFont();
	~BitmapFont();

	const BitmapGlyph* GlyphsBegin() const { return glyphs; }
	const BitmapGlyph* GlyphsEnd() const { return glyphs + glyphCount; }

	Vec2f GetTextureSize() const { return textureSize; }

	bool LoadFromBDF(const Buffer<char>& content);

	unsigned int GetTextureDriverId() const { return textureId; }
};
