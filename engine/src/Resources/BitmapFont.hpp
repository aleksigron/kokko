#pragma once

#include <cstddef>

#include "Core/StringView.hpp"

#include "Math/Rectangle.hpp"
#include "Math/Vec2.hpp"

class Allocator;
class TextureManager;

struct BitmapGlyph
{
	unsigned int codePoint;
	Vec2f size;
	Vec2f texturePosition;
};

class BitmapFont
{
private:
	Allocator* allocator;

	unsigned int textureId;
	Vec2f textureSize;

	size_t* glyphSkipList;
	static const size_t glyphSkipListStep = 16;

	BitmapGlyph* glyphs;
	unsigned int glyphCount;

	Vec2i genericGlyphSize;

	static size_t FindPrintable(ConstStringView string);
	static size_t FindUnprintable(ConstStringView string);
	static size_t FindSpacesInString(ConstStringView string, size_t* posOut, size_t maxPositions);
	static int ParseInt(ConstStringView string);
	static void ParseBitmapRow(ConstStringView line, unsigned int pixels, unsigned char* bitmapOut);
	static Vec2f CalculateTextureSize(int glyphCount, Vec2f glyphSize);
	static bool CompareGlyphCodePointAsc(const BitmapGlyph& lhs, const BitmapGlyph& rhs);

public:
	BitmapFont(Allocator* allocator);
	~BitmapFont();

	const BitmapGlyph* GetGlyph(unsigned int codePoint) const;

	int GetLineHeight() const { return genericGlyphSize.y; }
	int GetGlyphWidth() const { return genericGlyphSize.x; }

	unsigned int GetTextureDriverId() const { return textureId; }
	Vec2f GetTextureSize() const { return textureSize; }

	bool LoadFromBDF(TextureManager* textureManager, ConstStringView content);
};
