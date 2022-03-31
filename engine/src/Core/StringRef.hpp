#pragma once

#include <cstddef>
#include <cstdint>

struct StringRef
{
	const char* str;
	size_t len;

	// Create a empty StringRef instance
	StringRef();

	// Create a StringRef instance from char pointer and length
	StringRef(const char* string, size_t length);

	// Create a StringRef instance from a c-string
	explicit StringRef(const char* string);

	const char& operator[](size_t index) const { return str[index]; }

	// Does the other StringRef object reference the same area in memory
	bool ReferenceEquals(const StringRef& other) const;

	// Does the other StringRef object contain the same value
	bool ValueEquals(const StringRef& other) const;
	bool ValueEquals(const char* cstring) const;

	bool operator==(const StringRef& other) const;
	bool operator!=(const StringRef& other) const;

	// Clear this object
	void Clear();

	bool StartsWith(const StringRef& other) const;
	bool EndsWith(const StringRef& other) const;

	StringRef SubStr(size_t startPos, size_t length = 0) const;
	StringRef SubStrPos(size_t startPos, intptr_t endPos) const;

	intptr_t FindFirst(const StringRef& str, size_t startAt = 0) const;

	intptr_t FindFirstOf(const char* chars, size_t startAt = 0) const;
	intptr_t FindFirstNotOf(const char* chars, size_t startAt = 0) const;

	intptr_t FindLast(const StringRef& str) const;
};

namespace kokko
{
uint32_t Hash32(const StringRef& value, uint32_t seed);
}
