#pragma once

#include <cstddef>
#include <cstdint>

template <typename CharType>
struct StringRefBase
{
	CharType* str;
	size_t len;

	// Create a empty StringRef instance
	StringRefBase();

	// Create a StringRef instance from char pointer and length
	StringRefBase(CharType* string, size_t length);

	// Create a StringRef instance from a c-string
	explicit StringRefBase(CharType* string);

	CharType& operator[](size_t index) { return str[index]; }
	const CharType& operator[](size_t index) const { return str[index]; }

	// Does the other StringRef object reference the same area in memory
	bool ReferenceEquals(const StringRefBase<CharType>& other) const;

	// Does the other StringRef object contain the same value
	bool ValueEquals(const StringRefBase<CharType>& other) const;
	bool ValueEquals(const char* cstring) const;

	bool operator==(const StringRefBase<CharType>& other) const;
	bool operator!=(const StringRefBase<CharType>& other) const;

	// Clear this object
	void Clear();

	bool StartsWith(const StringRefBase<CharType>& other) const;
	bool EndsWith(const StringRefBase<CharType>& other) const;

	StringRefBase<CharType> SubStr(size_t startPos, size_t length = 0) const;
	StringRefBase<CharType> SubStrPos(size_t startPos, intptr_t endPos) const;

	intptr_t FindFirst(StringRefBase<const CharType> str, size_t startAt = 0) const;

	intptr_t FindFirstOf(const char* chars, size_t startAt = 0) const;
	intptr_t FindFirstNotOf(const char* chars, size_t startAt = 0) const;

	intptr_t FindLast(StringRefBase<const CharType> str) const;
};

using MutableStringRef = StringRefBase<char>;
using StringRef = StringRefBase<const char>;

namespace kokko
{
uint32_t Hash32(const StringRef& value, uint32_t seed);
}
