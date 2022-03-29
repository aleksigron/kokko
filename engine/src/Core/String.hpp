#pragma once

#include <cstddef>
#include <cstdint>

class Allocator;

struct StringRef;

namespace kokko
{

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

	const char* GetCStr() const { return GetData(); }

	char* GetData() { return string != nullptr ? string : reinterpret_cast<char*>(&length); }
	const char* GetData() const { return string != nullptr ? string : reinterpret_cast<const char*>(&length); }

	char* Begin() { return string; }
	const char* Begin() const { return string; }

	char* End() { return string + length; }
	const char* End() const { return string + length; }

	size_t GetLength() const { return length; }
	StringRef GetRef() const;

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

	void Replace(char replace, char with);

	// To gain access to allocator
	friend String operator+(const String& lhs, StringRef rhs);
	friend String operator+(StringRef lhs, const String& rhs);
};

String operator+(const String& lhs, StringRef rhs);
String operator+(StringRef lhs, const String& rhs);

String operator+(const String& lhs, const String& rhs);

String operator+(const String& lhs, const char* rhs);
String operator+(const char* lhs, const String& rhs);

String operator+(const String& lhs, char rhs);
String operator+(char lhs, const String& rhs);

bool operator==(const String& lhs, const String& rhs);
bool operator==(const String& lhs, const char* rhs);
bool operator==(const char* lhs, const String& rhs);
bool operator==(const String& lhs, StringRef rhs);
bool operator==(StringRef lhs, const String& rhs);

bool operator!=(const String& lhs, const String& rhs);
bool operator!=(const String& lhs, const char* rhs);
bool operator!=(const char* lhs, const String& rhs);
bool operator!=(const String& lhs, StringRef rhs);
bool operator!=(StringRef lhs, const String& rhs);

}
