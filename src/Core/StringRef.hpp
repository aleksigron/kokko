#pragma once

#include <cstdint>

struct StringRef
{
	const char* str;
	size_t len;

	// Create a empty StringRef instance
	StringRef() : str(nullptr), len(0) {}

	// Create a StringRef instance from char pointer and length
	StringRef(const char* string, size_t length) : str(string), len(length) {}

	// Create a StringRef instance from a c-string
	explicit StringRef(const char* string) : str(string)
	{
		while (*string != '\0')
		{
			++string;
		}

		len = static_cast<size_t>(string - str);
	}

	const char& operator[](size_t index) const { return str[index]; }

	// Does the other StringRef object reference the same area in memory
	bool ReferenceEquals(const StringRef& other) const
	{
		return this->str == other.str && this->len == other.len;
	}

	// Does the other StringRef object contain the same value
	bool ValueEquals(const StringRef& other) const;

	// Does the c-string contain the same value as this StringRef
	bool ValueEquals(const char* cstring) const;

	bool operator == (const StringRef& other) const
	{
		return this->ValueEquals(other);
	}

	bool operator != (const StringRef& other) const
	{
		return this->ValueEquals(other) == false;
	}

	// Return true if this->str is not null
	bool IsNonNull() const { return str != nullptr; }

	// Clear this object
	void Clear()
	{
		str = nullptr;
		len = 0;
	}

	bool StartsWith(const StringRef& other) const;
	bool EndsWith(const StringRef& other) const;

	StringRef SubStr(size_t startPos, size_t length = 0) const;
	StringRef SubStrPos(size_t startPos, intptr_t endPos) const;

	intptr_t FindFirst(const StringRef& str, size_t startAt = 0) const;

	intptr_t FindFirstOf(const char* chars, size_t startAt = 0) const;
	intptr_t FindFirstNotOf(const char* chars, size_t startAt = 0) const;
};
