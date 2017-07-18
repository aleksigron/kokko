#include "BitmapFont.hpp"

#include <cstdlib>
#include <cstring>

#include "Hash.hpp"
#include "AsciiStringUtil.hpp"

#include "Engine.hpp"
#include "App.hpp"
#include "Texture.hpp"
#include "ImageData.hpp"
#include "ResourceManager.hpp"
#include "Sort.hpp"

#include "IncludeOpenGL.hpp"

static bool CompareGlyphCodePointAsc(const BitmapGlyph& lhs, const BitmapGlyph& rhs)
{
	return lhs.codePoint < rhs.codePoint;
}

BitmapFont::BitmapFont() :
	textureId(0),
	glyphSkipList(nullptr),
	glyphs(nullptr),
	glyphCount(0)
{
}

BitmapFont::~BitmapFont()
{
	delete[] glyphs;
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

bool BitmapFont::LoadFromBDF(const Buffer<char>& content)
{
	using uint = unsigned int;
	using namespace AsciiStringUtil;

	StringRef unprocessed;
	unprocessed.str = content.Data();
	unprocessed.len = static_cast<uint>(content.Count());

	// Find end of first line

	StringRef line;

	uint lineEnd = FindUnprintable(unprocessed);

	if (lineEnd < unprocessed.len)
	{
		line.str = unprocessed.str;
		line.len = lineEnd;
	}

	int readGlyphs = -1;
	int readBitmapRows = -1;

	Vec2f glyphSize(0.0f, 0.0f);
	Vec2f glyphSafeSize(0.0f, 0.0f);

	Buffer<unsigned char> textureBuffer;
	Vec2i glyphsOnAxes(0, 0);

	uint spacesInLine[7];
	StringRef tokens[8];

	while (line.IsValid())
	{
		// Find spaces in the line

		uint spaceCount = FindSpacesInString(line, spacesInLine, 7);

		// Create token string references

		if (spaceCount > 0)
		{
			tokens[0].str = line.str;
			tokens[0].len = spacesInLine[0];

			for (uint i = 1; i < spaceCount; ++i)
			{
				uint tokenStart = spacesInLine[i - 1] + 1;
				tokens[i].str = line.str + tokenStart;
				tokens[i].len = spacesInLine[i] - tokenStart;
			}

			uint tokenStart = spacesInLine[spaceCount - 1] + 1;
			tokens[spaceCount].str = line.str + tokenStart;
			tokens[spaceCount].len = line.len - tokenStart;
		}
		else
		{
			tokens[0] = line;
		}

		// Calculate hash of first token, usually the keyword
		uint32_t hash = Hash::FNV1a_32(tokens[0].str, tokens[0].len);

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
						uint skipStep = glyphSkipListStep;
						uint skipListLength = glyphCount + ((skipStep - 1)) / skipStep;
						this->glyphSkipList = new uint[skipListLength];

						this->glyphs = new BitmapGlyph[glyphCount];
						this->glyphCount = glyphCount;

						textureSize = CalculateTextureSize(glyphCount, glyphSafeSize);
						int pixelCount = static_cast<int>(textureSize.x * textureSize.y);
						textureBuffer.Allocate(pixelCount);

						// Set buffer to zero so we don't have to worry about unused parts
						std::memset(textureBuffer.Data(), 0, textureBuffer.Count());

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
				int glyphRowWidth = currentGlyph->size.x;

				unsigned char* rowBuffer = textureBuffer.Data() + bufferOffset;

				ParseBitmapRow(line, glyphRowWidth, rowBuffer);

				++readBitmapRows;
			}
		}

		unprocessed.TrimBeginning(lineEnd);
		uint nextLineStart = FindPrintable(unprocessed);
		unprocessed.TrimBeginning(nextLineStart);

		lineEnd = FindUnprintable(unprocessed);

		if (lineEnd < unprocessed.len)
		{
			line.str = unprocessed.str;
			line.len = lineEnd;
		}
		else
		{
			line.Invalidate();
		}
	}

	if (readGlyphs > 0)
	{
		// Let's create a skip list

		// First make sure the glyphs are sorted by code point value
		InsertionSortPred(glyphs, glyphCount, CompareGlyphCodePointAsc);

		// Update skip values
		for (uint i = glyphSkipListStep - 1; i < glyphCount; i += glyphSkipListStep)
		{
			glyphSkipList[i / glyphSkipListStep] = glyphs[i].codePoint;
		}

		// Set last skip value
		uint skipIndex = (readGlyphs + (glyphSkipListStep - 1)) / glyphSkipListStep;
		glyphSkipList[skipIndex] = glyphs[readGlyphs - 1].codePoint;
	}

	if (textureBuffer.IsValid())
	{
		ImageData imageData;

		imageData.imageData = textureBuffer.Data();
		imageData.imageDataSize = textureBuffer.Count();

		imageData.imageSize = Vec2i(int(textureSize.x), int(textureSize.y));
		imageData.pixelFormat = GL_RED;
		imageData.componentDataType = GL_UNSIGNED_BYTE;

		Engine* engine = Engine::GetInstance();
		ResourceManager* rm = engine->GetResourceManager();

		Texture* texture = rm->CreateTexture();
		TextureOptions options;
		options.filter = TextureFilterMode::Nearest;
		texture->Upload_2D(imageData, options);

		textureId = texture->driverId;
		
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

void BitmapFont::ParseBitmapRow(StringRef line, unsigned int pixels, unsigned char* bitmapOut)
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
