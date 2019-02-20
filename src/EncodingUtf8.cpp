#include "EncodingUtf8.hpp"

unsigned int EncodingUtf8::EncodeCodepoint(unsigned int codepoint, char* utf8BytesOut)
{
	if (codepoint <= 0x7F)
	{
		utf8BytesOut[0] = codepoint;
		return 1;
	}
	else if (codepoint <= 0x7FF)
	{
		utf8BytesOut[0] = 0xc0 | (codepoint >> 6 & 0x1f);
		utf8BytesOut[1] = 0x80 | (codepoint & 0x3f);
		return 2;
	}
	else if (codepoint <= 0xFFFF)
	{
		utf8BytesOut[0] = 0xf0 | (codepoint >> 12 & 0x0f);
		utf8BytesOut[1] = 0x80 | (codepoint >> 6 & 0x3f);
		utf8BytesOut[2] = 0x80 | (codepoint & 0x3F);
		return 3;
	}
	else if (codepoint <= 0x10FFFF)
	{
		utf8BytesOut[0] = 0xf0 | (codepoint >> 18 & 0x07);
		utf8BytesOut[1] = 0x80 | (codepoint >> 12 & 0x3f);
		utf8BytesOut[2] = 0x80 | (codepoint >> 6 & 0x3f);
		utf8BytesOut[3] = 0x80 | (codepoint & 0x3f);
		return 4;
	}
	else
		return 0;
}

unsigned int EncodingUtf8::DecodeCodepoint(const char* input, unsigned int& codepointOut)
{
	char byte0 = input[0];
	unsigned int bytesDecoded = 0;

	if ((byte0 & 0x80) == 0x00) // First bit is 0
	{
		codepointOut = byte0;
		bytesDecoded = 1;
	}
	else if ((byte0 & 0xe0) == 0xc0) // First 3 bits are 110
	{
		codepointOut = (byte0 & 0x1f) << 6 | (input[1] & 0x3f);
		bytesDecoded = 2;
	}
	else if ((byte0 & 0xf0) == 0xe0) // First 4 bits are 1110
	{
		codepointOut = (byte0 & 0x0f) << 12 | (input[1] & 0x3f) << 6 | (input[2] & 0x3f);
		bytesDecoded = 3;
	}
	else if ((byte0 & 0xf8) == 0xf0) // First 5 bits are 11110
	{
		codepointOut = (byte0 & 0x07) << 18 | (input[1] & 0x3f) << 12 | (input[2] & 0x3f) << 6 | (input[3] & 0x3f);
		bytesDecoded = 4;
	}
	else // Not a valid first byte of character
	{
		bytesDecoded = 0;
	}

	return bytesDecoded;
}

unsigned int EncodingUtf8::CountCharacters(StringRef input)
{
	unsigned int count = 0;

	const char* itr = input.str;
	const char* end = itr + input.len;
	while (itr < end)
	{
		char c = *itr;

		if ((c & 0x80) == 0x00)
		{
			itr += 1;
			++count;
		}
		else if ((c & 0xe0) == 0xc0)
		{
			itr += 2;
			++count;
		}
		else if ((c & 0xf0) == 0xe0)
		{
			itr += 3;
			++count;
		}
		else if ((c & 0xf8) == 0xf0)
		{
			itr += 4;
			++count;
		}
		else // Not a valid first byte of character
		{
			itr += 1;
		}
	}

	return count;
}
