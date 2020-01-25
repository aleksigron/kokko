#pragma once

#include <cstddef>

#include "Core/StringRef.hpp"

class ImmutableString
{
private:
	char* string = nullptr;
	std::size_t length = 0;

public:
	ImmutableString();
	ImmutableString(const char* s);
	ImmutableString(const char* s, size_t len);
	ImmutableString(const ImmutableString& other);
	ImmutableString(ImmutableString&& other);

	~ImmutableString();

	ImmutableString& operator=(const ImmutableString& other);
	ImmutableString& operator=(ImmutableString&& other);

	const char* GetCstr() const { return this->string; }
	size_t GetLength() const { return this->length; }

	StringRef GetRef() const { return StringRef(string, length); }
};
