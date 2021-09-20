#pragma once

#include <cstddef>

#include "Core/StringRef.hpp"

class Allocator;

class String
{
private:
	Allocator* allocator;

	char* string;
	size_t length;
	size_t allocated;

	static size_t CalculateAllocationSize(size_t currentAllocated, size_t requiredSize);

public:
	String();
	explicit String(Allocator* allocator);
	String(const String& s);
	String(String&& s) noexcept;
	String(Allocator* allocator, const char* s);
	String(Allocator* allocator, StringRef s);

	~String();

	String& operator=(const String& s);
	String& operator=(String&& s) noexcept;

	void SetAllocator(Allocator* allocator);

	const char* GetCStr() const
	{
		if (string != nullptr)
			return string;
		else
		{
			// Return pointer to null character even when no memory is allocated
			return reinterpret_cast<const char*>(&length);
		}
	}

	char* GetData() { return string; }
	const char* GetData() const { return string; }

	char* Begin() { return string; }
	const char* Begin() const { return string; }

	char* End() { return string + length; }
	const char* End() const { return string + length; }

	size_t GetLength() const { return length; }
	StringRef GetRef() const { return StringRef(string, length); }

	char& At(size_t index) { return string[index]; }
	const char& At(size_t index) const { return string[index]; }

	char& operator[](size_t index) { return string[index]; }
	const char& operator[](size_t index) const { return string[index]; }

	String& operator+=(const String& append);

	void Append(StringRef s);
	void Append(const String& s);
	void Append(const char* s);
	void Append(char c);

	void Assign(StringRef s);
	void Assign(const String& s);
	void Assign(const char* s);

	void Reserve(size_t reserveLength);
	void Resize(size_t size);

	void Clear();

	// To gain access to allocator
	friend String operator+(const String& lhs, StringRef rhs);
};

String operator+(const String& lhs, StringRef rhs);
String operator+(StringRef lhs, const String& rhs);

String operator+(const String& lhs, const String& rhs);

String operator+(const String& lhs, const char* rhs);
String operator+(const char* lhs, const String& rhs);

bool operator==(const String& lhs, const String& rhs);
bool operator==(const String& lhs, const char* rhs);
bool operator==(const char* lhs, const String& rhs);

bool operator!=(const String& lhs, const String& rhs);
bool operator!=(const String& lhs, const char* rhs);
bool operator!=(const char* lhs, const String& rhs);
