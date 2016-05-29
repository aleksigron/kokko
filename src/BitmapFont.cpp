#include "BitmapFont.hpp"

#include <cstdlib>
#include <cstring>

#include "Hash.hpp"
#include "AsciiStringUtil.hpp"

#include "App.hpp"
#include "Texture.hpp"
#include "ImageData.hpp"
#include "ResourceManager.hpp"

#define GLFW_INCLUDE_GLCOREARB
#include "glfw/glfw3.h"

BitmapFont::BitmapFont() :
	textureId(0),
	glyphs(nullptr),
	glyphCount(0)
{

}

BitmapFont::~BitmapFont()
{

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

	Vec2f genericGlyphSize(0.0f, 0.0f);
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
					genericGlyphSize.x = float(ParseInt(tokens[1]));
					genericGlyphSize.y = float(ParseInt(tokens[2]));

					glyphSafeSize = genericGlyphSize + Vec2f(2.0f, 2.0f);
				}
					break;

				case "CHARS"_hash:
				{
					readGlyphs = 0;

					int glyphCount = ParseInt(tokens[1]);

					if (glyphCount > 0)
					{
						this->glyphs = new BitmapGlyph[glyphCount];
						this->glyphCount = glyphCount;

						textureSize = CalculateTextureSize(glyphCount, glyphSafeSize);
						textureBuffer.Allocate(static_cast<int>(textureSize.x * textureSize.y));

						// Set buffer to zero so we don't have to worry about unused parts
						std::memset(textureBuffer.Data(), 0, textureBuffer.Count());

						glyphsOnAxes.x = int(textureSize.x / glyphSafeSize.x);
						glyphsOnAxes.y = int(textureSize.y / glyphSafeSize.y);
					}
				}
					break;

				case "ENCODING"_hash:
				{
					currentGlyph->codePoint = ParseInt(tokens[1]);
				}
					break;

				case "BBX"_hash:
				{
					currentGlyph->size.x = float(ParseInt(tokens[1]));
					currentGlyph->size.y = float(ParseInt(tokens[2]));
					currentGlyph->drawOffset.x = float(ParseInt(tokens[3]));
					currentGlyph->drawOffset.y = float(ParseInt(tokens[4]));

					Vec2f* uv = &(currentGlyph->texturePosition);
					uv->x = (readGlyphs % glyphsOnAxes.x) * glyphSafeSize.x + 1.0f;
					uv->y = (readGlyphs / glyphsOnAxes.x) * glyphSafeSize.y + 1.0f;
				}
					break;

				case "DWIDTH"_hash:
				{
					currentGlyph->advance.x = float(ParseInt(tokens[1]));
					currentGlyph->advance.y = float(ParseInt(tokens[2]));
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

	if (textureBuffer.IsValid())
	{
		ImageData imageData;

		imageData.imageData = textureBuffer.Data();
		imageData.imageDataSize = textureBuffer.Count();

		imageData.imageSize = Vec2i(int(textureSize.x), int(textureSize.y));
		imageData.pixelFormat = GL_RED;
		imageData.componentDataType = GL_UNSIGNED_BYTE;

		ResourceManager* rm = App::GetResourceManager();
		Texture* texture = rm->CreateTexture();
		texture->Upload(imageData);
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
