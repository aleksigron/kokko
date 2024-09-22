#pragma once

#include <cstddef>
#include <cstdint>

namespace kokko
{

template <typename CharType>
struct StringView
{
	CharType* str;
	size_t len;

	// Create a empty ConstStringView instance
	StringView();

	// Create a ConstStringView instance from char pointer and length
	StringView(CharType* string, size_t length);

	// Create a ConstStringView instance from a c-string
	explicit StringView(CharType* string);

	CharType& operator[](size_t index) { return str[index]; }
	const CharType& operator[](size_t index) const { return str[index]; }

	// Does the other ConstStringView object reference the same area in memory
	bool ReferenceEquals(const StringView<CharType>& other) const;

	// Does the other ConstStringView object contain the same value
	bool ValueEquals(const StringView<CharType>& other) const;
	bool ValueEquals(const char* cstring) const;

	bool operator==(const StringView<CharType>& other) const;
	bool operator!=(const StringView<CharType>& other) const;

	// Clear this object
	void Clear();

	bool StartsWith(const StringView<CharType>& other) const;
	bool EndsWith(const StringView<CharType>& other) const;

	StringView<CharType> SubStr(size_t startPos, size_t length = 0) const;
	StringView<CharType> SubStrPos(size_t startPos, intptr_t endPos) const;

	intptr_t FindFirst(StringView<const CharType> str, size_t startAt = 0) const;

	intptr_t FindFirstOf(const char* chars, size_t startAt = 0) const;
	intptr_t FindFirstNotOf(const char* chars, size_t startAt = 0) const;

	intptr_t FindLast(StringView<const CharType> str) const;
};

using MutableStringView = StringView<char>;
using ConstStringView = StringView<const char>;

uint32_t HashValue32(const ConstStringView& value, uint32_t seed);

}
