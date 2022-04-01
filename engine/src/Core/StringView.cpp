#include "Core/StringView.hpp"

#include <cassert>
#include <cstring>

#include "doctest/doctest.h"

#include "Core/Hash.hpp"

doctest::String toString(const StringView<const char>& value)
{
	return doctest::String(value.str, static_cast<unsigned int>(value.len));
}

template <typename CharType>
StringView<CharType>::StringView() :
	str(nullptr), len(0)
{
}

template <typename CharType>
StringView<CharType>::StringView(CharType* string, size_t length) :
	str(string), len(length)
{
}

template <typename CharType>
StringView<CharType>::StringView(CharType* string) :
	str(string)
{
	while (*string != '\0')
		++string;

	len = static_cast<size_t>(string - str);
}

template <typename CharType>
bool StringView<CharType>::ReferenceEquals(const StringView<CharType>& other) const
{
	return this->str == other.str && this->len == other.len;
}

template <typename CharType>
bool StringView<CharType>::ValueEquals(const StringView<CharType>& other) const
{
	if (len != other.len || str == nullptr || other.str == nullptr)
		return false;

	for (unsigned i = 0; i < len; ++i)
		if (str[i] != other.str[i])
			return false;

	return true;
}

template <typename CharType>
bool StringView<CharType>::ValueEquals(const char* cstring) const
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

	// The strings match for the length of this StringView
	// If cstring too ends here, the strings are equal
	return cstring[i] == '\0';
}

template <typename CharType>
bool StringView<CharType>::operator==(const StringView<CharType>& other) const
{
	return this->ValueEquals(other);
}

template <typename CharType>
bool StringView<CharType>::operator!=(const StringView<CharType>& other) const
{
	return this->ValueEquals(other) == false;
}

TEST_CASE("StringView can test for equality")
{
	const char* str1 = "Test string";
	const char* str2 = "Test string";
	const char* str3 = "Test str";

	ConstStringView ref1(str1);
	ConstStringView ref2(str2);
	ConstStringView ref3(str3);
	ConstStringView ref4(str1, ref3.len);

	CHECK(ref1 == ref2);
	CHECK(ref1 != ref3);
	CHECK(ref1 != ConstStringView());
	CHECK(ConstStringView() != ConstStringView());
	CHECK(ref1.ValueEquals(ref2) == true);
	CHECK(ref1.ValueEquals(str2) == true);
	CHECK(ref1.ValueEquals(ref3) == false);
	CHECK(ref3.ValueEquals(ref4) == true);
	CHECK(ref3.ReferenceEquals(ref4) == false);
}

template <typename CharType>
void StringView<CharType>::Clear()
{
	str = nullptr;
	len = 0;
}

template <typename CharType>
bool StringView<CharType>::StartsWith(const StringView& other) const
{
	if (this->len < other.len)
		return false;

	for (unsigned int i = 0; i < other.len; ++i)
		if (this->str[i] != other.str[i])
			return false;

	return true;
}

template <typename CharType>
bool StringView<CharType>::EndsWith(const StringView& other) const
{
	if (len < other.len)
		return false;

	for (size_t idx = len - other.len, otherIdx = 0; idx < len; ++idx, ++otherIdx)
		if (str[idx] != other.str[otherIdx])
			return false;

	return true;
}

TEST_CASE("StringView can match string start and end")
{
	ConstStringView str("Test string");

	CHECK(str.StartsWith(ConstStringView("T")) == true);
	CHECK(str.StartsWith(ConstStringView("Test")) == true);
	CHECK(str.StartsWith(ConstStringView("est")) == false);

	CHECK(str.EndsWith(ConstStringView("g")) == true);
	CHECK(str.EndsWith(ConstStringView("ing")) == true);
	CHECK(str.EndsWith(ConstStringView("tin")) == false);
}

template <typename CharType>
StringView<CharType> StringView<CharType>::SubStr(size_t startPos, size_t length) const
{
	assert(startPos < len || (startPos == len && length == 0));

	if (length == 0)
		length = len - startPos;

	return StringView<CharType>(str + startPos, length);
}

template <typename CharType>
StringView<CharType> StringView<CharType>::SubStrPos(size_t startPos, intptr_t end) const
{
	assert(startPos < len || startPos == end);
	assert(end <= (intptr_t)len);

	size_t endPos = (end < 0) ? len + end : end;
	
	if (endPos < startPos)
		return StringView();

	return StringView<CharType>(str + startPos, endPos - startPos);
}

TEST_CASE("StringView can return substrings")
{
	ConstStringView testString("Test test string");

	CHECK(testString.SubStr(0) == testString);
	CHECK(testString.SubStr(1) != testString);
	CHECK(testString.SubStr(0, testString.len) == testString);
	CHECK(testString.SubStr(0, testString.len - 1) != testString);
	CHECK(testString.SubStr(0, 4) == ConstStringView("Test"));
	CHECK(testString.SubStr(5, 4) == ConstStringView("test"));
	CHECK(testString.SubStr(10) == ConstStringView("string"));

	CHECK(testString.SubStrPos(0, testString.len) == testString);
	CHECK(testString.SubStrPos(0, -1) != testString);
	CHECK(testString.SubStrPos(0, 1) != testString);
	CHECK(testString.SubStrPos(0, 4) == ConstStringView("Test"));
	CHECK(testString.SubStrPos(0, -12) == ConstStringView("Test"));
	CHECK(testString.SubStrPos(5, 9) == ConstStringView("test"));
	CHECK(testString.SubStrPos(5, -7) == ConstStringView("test"));
}

template <typename CharType>
intptr_t StringView<CharType>::FindFirst(StringView<const CharType> find, size_t startAt) const
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

TEST_CASE("StringView can find substrings")
{
	ConstStringView str("Test string");

	CHECK(str.FindFirst(ConstStringView("Test")) == 0);
	CHECK(str.FindFirst(ConstStringView("Test"), 1) < 0);
	CHECK(str.FindFirst(ConstStringView("ing")) == 8);
	CHECK(str.FindFirst(ConstStringView("ing"), 8) == 8);
}

template <typename CharType>
intptr_t StringView<CharType>::FindFirstOf(const char* chars, size_t startAt) const
{
	size_t charCount = std::strlen(chars);

	for (size_t sourceIdx = startAt; sourceIdx < len; ++sourceIdx)
		for (size_t charIdx = 0; charIdx < charCount; ++charIdx)
			if (str[sourceIdx] == chars[charIdx])
				return sourceIdx;

	return -1;
}

template <typename CharType>
intptr_t StringView<CharType>::FindFirstNotOf(const char* chars, size_t startAt) const
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

TEST_CASE("StringView can find chars")
{
	ConstStringView str("Test string");

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
intptr_t StringView<CharType>::FindLast(StringView<const CharType> find) const
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

TEST_CASE("StringView can find last substring")
{
	ConstStringView str("Test str str");

	CHECK(str.FindLast(ConstStringView("Test")) == 0);
	CHECK(str.FindLast(ConstStringView("str")) == 9);
	CHECK(str.FindLast(ConstStringView("str ")) == 5);
	CHECK(str.FindLast(ConstStringView("ing")) < 0);
}

namespace kokko
{
uint32_t Hash32(const ConstStringView& value, uint32_t seed)
{
	return Hash32(value.str, value.len, seed);
}
}

template class StringView<char>;
template class StringView<const char>;
