#pragma once

#include "Core/StringRef.hpp"

class String
{
public:
	using SizeType = unsigned int;

private:
	char* string;
	SizeType length;
	SizeType allocated;

	static SizeType CalculateAllocationSize(SizeType currentAllocated, SizeType requiredSize);

public:
	String();
	String(const String& s);
	String(String&& s);
	explicit String(const char* s);
	explicit String(StringRef s);

	~String();

	String& operator=(const String& s);
	String& operator=(String&& s);

	const char* GetCStr() const { return string; }

	char* Begin() { return string; }
	const char* Begin() const { return string; }

	char* End() { return string + length; }
	const char* End() const { return string + length; }

	SizeType GetLength() const { return length; }
	StringRef GetRef() const { return StringRef(string, length); }

	char& At(SizeType index) { return string[index]; }
	const char& At(SizeType index) const { return string[index]; }

	char& operator[](SizeType index) { return string[index]; }
	const char& operator[](SizeType index) const { return string[index]; }

	String& operator+=(const String& append);

	void Append(StringRef s);
	void Append(const String& s);
	void Append(const char* s);
	void Append(char c);

	void Reserve(SizeType reserveLength);
	void Resize(SizeType size);

	void Clear();
};

String operator+(const String& lhs, StringRef rhs);
String operator+(StringRef lhs, const String& rhs);

String operator+(const String& lhs, const String& rhs);

String operator+(const String& lhs, const char* rhs);
String operator+(const char* lhs, const String& rhs);
