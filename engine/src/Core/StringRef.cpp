#include "Core/StringRef.hpp"

#include <cassert>
#include <cstring>

#include "doctest/doctest.h"

#include "Core/Hash.hpp"

doctest::String toString(const StringRefBase<const char>& value)
{
	return doctest::String(value.str, static_cast<unsigned int>(value.len));
}

template <typename CharType>
StringRefBase<CharType>::StringRefBase() :
	str(nullptr), len(0)
{
}

template <typename CharType>
StringRefBase<CharType>::StringRefBase(CharType* string, size_t length) :
	str(string), len(length)
{
}

template <typename CharType>
StringRefBase<CharType>::StringRefBase(CharType* string) :
	str(string)
{
	while (*string != '\0')
		++string;

	len = static_cast<size_t>(string - str);
}

template <typename CharType>
bool StringRefBase<CharType>::ReferenceEquals(const StringRefBase<CharType>& other) const
{
	return this->str == other.str && this->len == other.len;
}

template <typename CharType>
bool StringRefBase<CharType>::ValueEquals(const StringRefBase<CharType>& other) const
{
	if (len != other.len || str == nullptr || other.str == nullptr)
		return false;

	for (unsigned i = 0; i < len; ++i)
		if (str[i] != other.str[i])
			return false;

	return true;
}

template <typename CharType>
bool StringRefBase<CharType>::ValueEquals(const char* cstring) const
{
	if (str == nullptr || cstring == nullptr)
		return false;

	size_t i = 0;
	while (i < len)
	{
		if (cstring[i] == '\0' || str[i] != cstring[i])
			return false;

		++i;
	}

	// The strings match for the length of this StringRefBase
	// If cstring too ends here, the strings are equal
	return cstring[i] == '\0';
}

template <typename CharType>
bool StringRefBase<CharType>::operator==(const StringRefBase<CharType>& other) const
{
	return this->ValueEquals(other);
}

template <typename CharType>
bool StringRefBase<CharType>::operator!=(const StringRefBase<CharType>& other) const
{
	return this->ValueEquals(other) == false;
}

TEST_CASE("StringRefBase can test for equality")
{
	const char* str1 = "Test string";
	const char* str2 = "Test string";
	const char* str3 = "Test str";

	StringRef ref1(str1);
	StringRef ref2(str2);
	StringRef ref3(str3);
	StringRef ref4(str1, ref3.len);

	CHECK(ref1 == ref2);
	CHECK(ref1 != ref3);
	CHECK(ref1 != StringRef());
	CHECK(StringRef() != StringRef());
	CHECK(ref1.ValueEquals(ref2) == true);
	CHECK(ref1.ValueEquals(str2) == true);
	CHECK(ref1.ValueEquals(ref3) == false);
	CHECK(ref3.ValueEquals(ref4) == true);
	CHECK(ref3.ReferenceEquals(ref4) == false);
}

template <typename CharType>
void StringRefBase<CharType>::Clear()
{
	str = nullptr;
	len = 0;
}

template <typename CharType>
bool StringRefBase<CharType>::StartsWith(const StringRefBase& other) const
{
	if (this->len < other.len)
		return false;

	for (unsigned int i = 0; i < other.len; ++i)
		if (this->str[i] != other.str[i])
			return false;

	return true;
}

template <typename CharType>
bool StringRefBase<CharType>::EndsWith(const StringRefBase& other) const
{
	if (len < other.len)
		return false;

	for (size_t idx = len - other.len, otherIdx = 0; idx < len; ++idx, ++otherIdx)
		if (str[idx] != other.str[otherIdx])
			return false;

	return true;
}

TEST_CASE("StringRefBase can match string start and end")
{
	StringRef str("Test string");

	CHECK(str.StartsWith(StringRef("T")) == true);
	CHECK(str.StartsWith(StringRef("Test")) == true);
	CHECK(str.StartsWith(StringRef("est")) == false);

	CHECK(str.EndsWith(StringRef("g")) == true);
	CHECK(str.EndsWith(StringRef("ing")) == true);
	CHECK(str.EndsWith(StringRef("tin")) == false);
}

template <typename CharType>
StringRefBase<CharType> StringRefBase<CharType>::SubStr(size_t startPos, size_t length) const
{
	assert(startPos < len || (startPos == len && length == 0));

	if (length == 0)
		length = len - startPos;

	return StringRefBase<CharType>(str + startPos, length);
}

template <typename CharType>
StringRefBase<CharType> StringRefBase<CharType>::SubStrPos(size_t startPos, intptr_t end) const
{
	assert(startPos < len || startPos == end);
	assert(end <= (intptr_t)len);

	size_t endPos = (end < 0) ? len + end : end;
	
	if (endPos < startPos)
		return StringRefBase();

	return StringRefBase<CharType>(str + startPos, endPos - startPos);
}

TEST_CASE("StringRefBase can return substrings")
{
	StringRef testString("Test test string");

	CHECK(testString.SubStr(0) == testString);
	CHECK(testString.SubStr(1) != testString);
	CHECK(testString.SubStr(0, testString.len) == testString);
	CHECK(testString.SubStr(0, testString.len - 1) != testString);
	CHECK(testString.SubStr(0, 4) == StringRef("Test"));
	CHECK(testString.SubStr(5, 4) == StringRef("test"));
	CHECK(testString.SubStr(10) == StringRef("string"));

	CHECK(testString.SubStrPos(0, testString.len) == testString);
	CHECK(testString.SubStrPos(0, -1) != testString);
	CHECK(testString.SubStrPos(0, 1) != testString);
	CHECK(testString.SubStrPos(0, 4) == StringRef("Test"));
	CHECK(testString.SubStrPos(0, -12) == StringRef("Test"));
	CHECK(testString.SubStrPos(5, 9) == StringRef("test"));
	CHECK(testString.SubStrPos(5, -7) == StringRef("test"));
}

template <typename CharType>
intptr_t StringRefBase<CharType>::FindFirst(StringRefBase<const CharType> find, size_t startAt) const
{
	for (size_t sourceIdx = startAt; sourceIdx < len; ++sourceIdx)
	{
		if (sourceIdx + find.len > len)
			break;

		bool match = true;

		for (size_t findIdx = 0; findIdx < find.len; ++findIdx)
		{
			if (str[sourceIdx + findIdx] != find.str[findIdx])
			{
				match = false;
				break;
			}
		}

		if (match)
			return static_cast<intptr_t>(sourceIdx);
	}

	return -1;
}

TEST_CASE("StringRefBase can find substrings")
{
	StringRef str("Test string");

	CHECK(str.FindFirst(StringRef("Test")) == 0);
	CHECK(str.FindFirst(StringRef("Test"), 1) < 0);
	CHECK(str.FindFirst(StringRef("ing")) == 8);
	CHECK(str.FindFirst(StringRef("ing"), 8) == 8);
}

template <typename CharType>
intptr_t StringRefBase<CharType>::FindFirstOf(const char* chars, size_t startAt) const
{
	size_t charCount = std::strlen(chars);

	for (size_t sourceIdx = startAt; sourceIdx < len; ++sourceIdx)
		for (size_t charIdx = 0; charIdx < charCount; ++charIdx)
			if (str[sourceIdx] == chars[charIdx])
				return sourceIdx;

	return -1;
}

template <typename CharType>
intptr_t StringRefBase<CharType>::FindFirstNotOf(const char* chars, size_t startAt) const
{
	size_t charCount = std::strlen(chars);

	for (size_t sourceIdx = startAt; sourceIdx < len; ++sourceIdx)
	{
		bool match = false;

		for (size_t charIdx = 0; charIdx < charCount; ++charIdx)
			if (str[sourceIdx] == chars[charIdx])
				match = true;

		if (match == false)
			return sourceIdx;
	}

	return -1;
}

TEST_CASE("StringRefBase can find chars")
{
	StringRef str("Test string");

	CHECK(str.FindFirstOf("a") < 0);
	CHECK(str.FindFirstOf("T", 1) < 0);
	CHECK(str.FindFirstOf("Te", 2) < 0);
	CHECK(str.FindFirstOf("T") == 0);
	CHECK(str.FindFirstOf("tseT") == 0);
	CHECK(str.FindFirstOf("Te", 1) == 1);
	CHECK(str.FindFirstOf("g") == 10);
	CHECK(str.FindFirstOf("ing", 10) == 10);

	CHECK(str.FindFirstNotOf("Test ring") < 0);
	CHECK(str.FindFirstNotOf("e") == 0);
	CHECK(str.FindFirstNotOf("T") == 1);
	CHECK(str.FindFirstNotOf("Test") == 4);
	CHECK(str.FindFirstNotOf("Test ") == 7);
	CHECK(str.FindFirstNotOf("str", 5) == 8);
	CHECK(str.FindFirstNotOf("g", 10) < 0);
	CHECK(str.FindFirstNotOf("T", 10) == 10);
}

template <typename CharType>
intptr_t StringRefBase<CharType>::FindLast(StringRefBase<const CharType> find) const
{
	for (intptr_t sourceIdx = len - find.len; sourceIdx >= 0; --sourceIdx)
	{
		bool match = true;

		for (intptr_t findIdx = 0; findIdx < find.len; ++findIdx)
		{
			if (str[sourceIdx + find.len - findIdx - 1] != find.str[find.len - findIdx - 1])
			{
				match = false;
				break;
			}
		}

		if (match)
			return sourceIdx;
	}

	return -1;
}

TEST_CASE("StringRefBase can find last substring")
{
	StringRef str("Test str str");

	CHECK(str.FindLast(StringRef("Test")) == 0);
	CHECK(str.FindLast(StringRef("str")) == 9);
	CHECK(str.FindLast(StringRef("str ")) == 5);
	CHECK(str.FindLast(StringRef("ing")) < 0);
}

namespace kokko
{
uint32_t Hash32(const StringRef& value, uint32_t seed)
{
	return Hash32(value.str, value.len, seed);
}
}

template class StringRefBase<char>;
template class StringRefBase<const char>;
