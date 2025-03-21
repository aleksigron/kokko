#include "Resources/BitmapFont.hpp"

#include <cstdlib>
#include <cstring>

#include "Core/Array.hpp"
#include "Core/Core.hpp"
#include "Core/Hash.hpp"
#include "Core/Sort.hpp"

#include "Engine/Engine.hpp"

#include "Resources/ImageData.hpp"
#include "Resources/TextureManager.hpp"

#include "System/IncludeOpenGL.hpp"

namespace kokko
{

BitmapFont::BitmapFont(Allocator* allocator) :
	allocator(allocator),
	textureId(0),
	glyphSkipList(nullptr),
	glyphs(nullptr),
	glyphCount(0)
{
}

BitmapFont::~BitmapFont()
{
	allocator->Deallocate(glyphs);
	allocator->Deallocate(glyphSkipList);
}

const BitmapGlyph* BitmapFont::GetGlyph(unsigned int codePoint) const
{
	unsigned int skipStep = glyphSkipListStep;
	unsigned int skipListLength = glyphCount + ((skipStep - 1)) / skipStep;

	for (unsigned int i = 0; i < skipListLength; ++i)
	{
		if (glyphSkipList[i] >= codePoint)
		{
			const BitmapGlyph* itr = glyphs + (i * skipStep);
			const BitmapGlyph* end = itr + skipStep;

			for (; itr != end; ++itr)
			{
				if (itr->codePoint == codePoint)
				{
					return itr;
				}
			}
		}
	}

	return nullptr;
}

bool BitmapFont::LoadFromBDF(kokko::TextureManager* textureManager, kokko::ConstStringView content)
{
	KOKKO_PROFILE_FUNCTION();

	kokko::ConstStringView unprocessed = content;

	// Find end of first line

	kokko::ConstStringView line;

	size_t lineEnd = FindUnprintable(unprocessed);

	if (lineEnd < unprocessed.len)
	{
		line.str = unprocessed.str;
		line.len = lineEnd;
	}

	int readGlyphs = -1;
	int readBitmapRows = -1;

	Vec2f glyphSize(0.0f, 0.0f);
	Vec2f glyphSafeSize(0.0f, 0.0f);

	kokko::Array<unsigned char> textureBuffer(allocator);
	Vec2i glyphsOnAxes(0, 0);

	size_t spacesInLine[7];
	kokko::ConstStringView tokens[8];

	while (line.str != nullptr)
	{
		// Find spaces in the line

		size_t spaceCount = FindSpacesInString(line, spacesInLine, 7);

		// Create token string references

		if (spaceCount > 0)
		{
			tokens[0].str = line.str;
			tokens[0].len = spacesInLine[0];

			for (size_t i = 1; i < spaceCount; ++i)
			{
				size_t tokenStart = spacesInLine[i - 1] + 1;
				tokens[i].str = line.str + tokenStart;
				tokens[i].len = spacesInLine[i] - tokenStart;
			}

			size_t tokenStart = spacesInLine[spaceCount - 1] + 1;
			tokens[spaceCount].str = line.str + tokenStart;
			tokens[spaceCount].len = line.len - tokenStart;
		}
		else
		{
			tokens[0] = line;
		}

		// Calculate hash of first token, usually the keyword
		uint32_t hash = kokko::HashString(tokens[0].str, tokens[0].len);

		BitmapGlyph* currentGlyph = this->glyphs + readGlyphs;

		if (readBitmapRows < 0)
		{
			switch (hash)
			{
			case "FONTBOUNDINGBOX"_hash:
			{
				genericGlyphSize.x = ParseInt(tokens[1]);
				genericGlyphSize.y = ParseInt(tokens[2]);

				glyphSize.x = static_cast<float>(genericGlyphSize.x);
				glyphSize.y = static_cast<float>(genericGlyphSize.y);

				glyphSafeSize = glyphSize + Vec2f(2.0f, 2.0f);
			}
			break;

			case "CHARS"_hash:
			{
				readGlyphs = 0;

				int glyphCount = ParseInt(tokens[1]);

				if (glyphCount > 0)
				{
					size_t skipStep = glyphSkipListStep;
					size_t skipListLength = glyphCount + ((skipStep - 1)) / skipStep;

					size_t skipListSize = skipListLength * sizeof(size_t);
					this->glyphSkipList = static_cast<size_t*>(allocator->Allocate(skipListSize, "BitmapFont.glyphSkiplist"));

					size_t glyphsSize = glyphCount * sizeof(BitmapGlyph);
					this->glyphs = static_cast<BitmapGlyph*>(allocator->Allocate(glyphsSize, "BitmapFont.glyphs"));

					this->glyphCount = glyphCount;

					textureSize = CalculateTextureSize(glyphCount, glyphSafeSize);
					int pixelCount = static_cast<int>(textureSize.x * textureSize.y);
					textureBuffer.Resize(pixelCount);

					// Set buffer to zero so we don't have to worry about unused parts
					std::memset(textureBuffer.GetData(), 0, textureBuffer.GetCount());

					glyphsOnAxes.x = int(textureSize.x / glyphSafeSize.x);
					glyphsOnAxes.y = int(textureSize.y / glyphSafeSize.y);
				}
			}
			break;

			case "ENCODING"_hash:
				currentGlyph->codePoint = ParseInt(tokens[1]);
				break;

			case "BBX"_hash:
			{
				currentGlyph->size.x = float(ParseInt(tokens[1]));
				currentGlyph->size.y = float(ParseInt(tokens[2]));

				int col = readGlyphs % glyphsOnAxes.x;
				int row = readGlyphs / glyphsOnAxes.x;

				Vec2f* uv = &(currentGlyph->texturePosition);
				uv->x = col * glyphSafeSize.x + 1.0f;
				uv->y = row * glyphSafeSize.y + 1.0f;
			}
			break;

			case "BITMAP"_hash:
			{
				readBitmapRows = 0;
			}
			break;

			default:
				break;
			}
		}
		else // readingBitmap >= 0
		{
			if (hash == "ENDCHAR"_hash)
			{
				readBitmapRows = -1;
				++readGlyphs;
			}
			else // This is actual bitmap data row
			{
				const Vec2f& cellPosition = currentGlyph->texturePosition;

				Vec2i rowPos;
				rowPos.x = static_cast<int>(cellPosition.x);
				rowPos.y = static_cast<int>(cellPosition.y) + readBitmapRows;

				int bufferOffset = rowPos.y * static_cast<int>(textureSize.x) + rowPos.x;
				int glyphRowWidth = static_cast<int>(currentGlyph->size.x);

				unsigned char* rowBuffer = textureBuffer.GetData() + bufferOffset;

				ParseBitmapRow(line, glyphRowWidth, rowBuffer);

				++readBitmapRows;
			}
		}

		unprocessed = unprocessed.SubStr(lineEnd);
		size_t nextLineStart = FindPrintable(unprocessed);
		unprocessed = unprocessed.SubStr(nextLineStart);

		lineEnd = FindUnprintable(unprocessed);

		if (lineEnd < unprocessed.len)
		{
			line.str = unprocessed.str;
			line.len = lineEnd;
		}
		else
		{
			line.Clear();
		}
	}

	if (readGlyphs > 0)
	{
		// Let's create a skip list

		// First make sure the glyphs are sorted by code point value
		InsertionSortPred(glyphs, glyphCount, CompareGlyphCodePointAsc);

		// Update skip values
		for (size_t i = glyphSkipListStep - 1; i < glyphCount; i += glyphSkipListStep)
		{
			glyphSkipList[i / glyphSkipListStep] = glyphs[i].codePoint;
		}

		// Set last skip value
		size_t skipIndex = (readGlyphs + (glyphSkipListStep - 1)) / glyphSkipListStep;
		glyphSkipList[skipIndex] = glyphs[readGlyphs - 1].codePoint;
	}

	if (textureBuffer.GetData() != nullptr)
	{
		ImageData imageData;

		imageData.imageData = textureBuffer.GetData();
		imageData.imageDataSize = textureBuffer.GetCount();

		imageData.imageSize = Vec2i(int(textureSize.x), int(textureSize.y));
		imageData.pixelFormat = RenderTextureBaseFormat::R;
		imageData.componentDataType = RenderTextureDataType::UnsignedByte;

		kokko::TextureId id = textureManager->CreateTexture();

		textureManager->Upload_2D(id, imageData, false);

		textureId = textureManager->GetTextureData(id).textureObjectId;

		return true;
	}
	else
	{
		return false;
	}
}

Vec2f BitmapFont::CalculateTextureSize(int glyphCount, Vec2f glyphSize)
{
	Vec2f result(32.0f, 16.0f);
	Vec2i glyphs(0, 0);

	while (glyphs.x * glyphs.y < glyphCount)
	{
		if (result.x > result.y)
			result.y = result.x;
		else
			result.x = result.x * 2.0f;

		glyphs.x = static_cast<int>(result.x / glyphSize.x);
		glyphs.y = static_cast<int>(result.y / glyphSize.y);
	}

	return result;
}

void BitmapFont::ParseBitmapRow(kokko::ConstStringView line, unsigned int pixels, unsigned char* bitmapOut)
{
	for (unsigned int charIndex = 0; charIndex < line.len; ++charIndex)
	{
		char c = line.str[charIndex];
		unsigned char value = 0;

		if (c >= 'A' && c <= 'F')
			value = c - 55;
		else if (c >= '0' && c <= '9')
			value = c - 48;

		for (unsigned int j = 0; j < 4 && pixels > 0; ++j, --pixels)
		{
			unsigned int pixel = (value & (0x08 >> j)) * 255;
			bitmapOut[charIndex * 4 + j] = pixel;
		}
	}
}

size_t BitmapFont::FindPrintable(kokko::ConstStringView string)
{
	const char* itr = string.str;
	const char* end = string.str + string.len;
	for (; itr != end; ++itr)
	{
		char c = *itr;

		if (c < 127 && c > 31)
			return static_cast<unsigned int>(itr - string.str);
	}

	return string.len;
}

size_t BitmapFont::FindUnprintable(kokko::ConstStringView string)
{
	const char* itr = string.str;
	const char* end = string.str + string.len;
	for (; itr != end; ++itr)
	{
		char c = *itr;

		if (c >= 127 || c <= 31)
			return static_cast<unsigned int>(itr - string.str);
	}

	return string.len;
}

size_t BitmapFont::FindSpacesInString(kokko::ConstStringView string, size_t* posOut, size_t maxPositions)
{
	unsigned int foundCount = 0;

	const char* itr = string.str;
	const char* end = string.str + string.len;
	for (; itr != end && foundCount < maxPositions; ++itr)
	{
		if (*itr == ' ')
		{
			posOut[foundCount] = static_cast<unsigned int>(itr - string.str);
			++foundCount;
		}
	}

	return foundCount;
}

int BitmapFont::ParseInt(kokko::ConstStringView string)
{
	int result = 0;
	int sign = 1;

	const char* itr = string.str;
	const char* end = string.str + string.len;

	if (*itr == '-')
	{
		sign = -1;
		++itr;
	}

	for (; itr != end; ++itr)
	{
		result = result * 10 + (*itr - '0');
	}

	return result * sign;
}

bool BitmapFont::CompareGlyphCodePointAsc(const BitmapGlyph& lhs, const BitmapGlyph& rhs)
{
	return lhs.codePoint < rhs.codePoint;
}

} // namespace kokko
