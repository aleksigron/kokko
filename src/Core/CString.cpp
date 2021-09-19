#include "Core/CString.hpp"

#include <cassert>
#include <cstring>

#include "doctest/doctest.h"

#include "Core/EncodingUtf8.hpp"

static void ClearToUtf8Boundary()
{

}

void StringCopyN(char* destination, const char* source, size_t destBufferCount)
{
	assert(destBufferCount > 0);

	size_t i = 0;

	for (; i < destBufferCount; ++i)
	{
		char byte = source[i];
		destination[i] = byte;

		if (byte == '\0')
			break;
	}

	// String needs to be truncated
	if (i == destBufferCount)
	{
		// Make sure no UTF-8 characters are broken up

		StringRef destRef(destination, i);
		size_t lastCharPos = EncodingUtf8::FindLastCharacter(destRef);

		// We found a valid character
		if (lastCharPos != destRef.len)
		{
			uint32_t codepoint;

			// Check how many bytes the character takes
			size_t codepointBytes = EncodingUtf8::DecodeCodepoint(&destination[lastCharPos], codepoint);

			if (lastCharPos + codepointBytes >= destBufferCount)
			{
				destination[lastCharPos] = '\0';
			}
			else
			{
				destination[destBufferCount - 1] = '\0';
			}
		}
		else // Couldn't find any valid characters
		{
			destination[0] = '\0';
		}
	}
}

bool StringIsEmpty(const char* str)
{
	return str[0] == '\0';
}

void SetEmptyString(char* str)
{
	str[0] = '\0';
}

TEST_CASE("StringCopyN")
{
	char dest[8];

	StringCopyN(dest, "Str", sizeof(dest));
	CHECK(dest[0] == 'S');
	CHECK(dest[1] == 't');
	CHECK(dest[2] == 'r');
	CHECK(dest[3] == '\0');

	memset(dest, 0, sizeof(dest));

	StringCopyN(dest, "StrStrStr", sizeof(dest));
	CHECK(dest[6] == 'S');
	CHECK(dest[7] == '\0');

	memset(dest, 0, sizeof(dest));

	char utf8buf[] = u8"Tëstiä";

	StringCopyN(dest, utf8buf, sizeof(dest));
	CHECK(dest[5] == utf8buf[5]);
	CHECK(dest[6] == '\0');
}

TEST_CASE("StringCopySafe")
{
	char dest[8];

	StringCopySafe(dest, "Str");
	CHECK(dest[0] == 'S');
	CHECK(dest[1] == 't');
	CHECK(dest[2] == 'r');
	CHECK(dest[3] == '\0');

	memset(dest, 0, sizeof(dest));

	StringCopySafe(dest, "StrStrStr");
	CHECK(dest[6] == 'S');
	CHECK(dest[7] == '\0');

	memset(dest, 0, sizeof(dest));

	char utf8buf[] = u8"Tëstiä";

	StringCopySafe(dest, utf8buf);
	CHECK(dest[5] == utf8buf[5]);
	CHECK(dest[6] == '\0');
}

TEST_CASE("StringIsEmpty")
{
	CHECK(StringIsEmpty("") == true);
	CHECK(StringIsEmpty("A") == false);

	char buf0[4] = { '\0', '\0', '\0', '\0' };
	CHECK(StringIsEmpty(buf0) == true);

	char buf1[4] = { 'A', '\0', '\0', '\0' };
	CHECK(StringIsEmpty(buf1) == false);
}
