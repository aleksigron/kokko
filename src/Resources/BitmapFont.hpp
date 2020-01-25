#pragma once

#include "Math/Rectangle.hpp"
#include "Math/Vec2.hpp"

#include "Core/StringRef.hpp"
#include "Core/Buffer.hpp"

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

	Vec2i genericGlyphSize;

	static unsigned int FindPrintable(StringRef string);
	static unsigned int FindUnprintable(StringRef string);
	static unsigned int FindSpacesInString(StringRef string, unsigned int* posOut, unsigned int maxPositions);
	static int ParseInt(StringRef string);
	static void ParseBitmapRow(StringRef line, unsigned int pixels, unsigned char* bitmapOut);
	static Vec2f CalculateTextureSize(int glyphCount, Vec2f glyphSize);
	static bool CompareGlyphCodePointAsc(const BitmapGlyph& lhs, const BitmapGlyph& rhs);

public:
	BitmapFont();
	~BitmapFont();

	const BitmapGlyph* GetGlyph(unsigned int codePoint) const;

	int GetLineHeight() const { return genericGlyphSize.y; }
	int GetGlyphWidth() const { return genericGlyphSize.x; }

	unsigned int GetTextureDriverId() const { return textureId; }
	Vec2f GetTextureSize() const { return textureSize; }

	bool LoadFromBDF(const Buffer<char>& content);
};
