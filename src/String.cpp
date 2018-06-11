#include "String.hpp"

#include <cstring>

String::String() : string(nullptr), length(0), allocated(0)
{
}

String::String(const String& s)
{
	length = s.GetLength();
	allocated = CalculateAllocationSize(0, length);

	if (allocated > 0)
	{
		string = new char[allocated + 1];
		std::memcpy(string, s.Begin(), length);
		string[length] = '\0';
	}
	else
		string = nullptr;
}

String::String(String&& s)
{
	string = s.string;
	length = s.length;
	allocated = s.allocated;

	s.string = nullptr;
	s.length = 0;
	s.allocated = 0;
}

String::String(const char* s)
{
	length = std::strlen(s);
	allocated = CalculateAllocationSize(0, length);

	if (allocated > 0)
	{
		string = new char[allocated + 1];
		std::memcpy(string, s, length);
		string[length] = '\0';
	}
	else
		string = nullptr;
}

String::String(StringRef s)
{
	length = s.len;
	allocated = CalculateAllocationSize(0, length);

	if (allocated > 0)
	{
		string = new char[allocated + 1];
		std::memcpy(string, s.str, length);
		string[length] = '\0';
	}
	else
		string = nullptr;
}

String::~String()
{
	delete[] string;
}

String& String::operator=(const String& s)
{
	SizeType newLength = s.GetLength();

	if (newLength > allocated)
	{
		delete[] string;

		allocated = CalculateAllocationSize(allocated, newLength);
		string = new char[allocated + 1];
	}

	std::memcpy(string, s.Begin(), newLength);
	string[newLength] = '\0';
	length = newLength;

	return *this;
}

String& String::operator=(String&& s)
{
	delete[] string;

	string = s.string;
	length = s.length;
	allocated = s.allocated;

	s.string = nullptr;
	s.length = 0;
	s.allocated = 0;

	return *this;
}

String& String::operator+=(const String& append)
{
	this->Append(append.GetRef());

	return *this;
}

void String::Append(StringRef s)
{
	SizeType requiredLength = length + s.len;

	if (allocated < requiredLength)
	{
		SizeType newAllocated = CalculateAllocationSize(allocated, requiredLength);

		char* newString = new char[newAllocated + 1];
		std::memcpy(newString, string, length);
		delete[] string;

		string = newString;
		allocated = newAllocated;
	}

	std::memcpy(string + length, s.str, s.len);
	string[requiredLength] = '\0';
	length = requiredLength;
}

void String::Append(const String& s)
{
	this->Append(s.GetRef());
}

void String::Append(const char* s)
{
	this->Append(StringRef(s));
}

void String::Append(char c)
{
	if (allocated < length + 1)
	{
		SizeType newAllocated = CalculateAllocationSize(this->allocated, length + 1);

		char* newString = new char[newAllocated + 1];
		std::memcpy(newString, string, length);
		delete[] string;

		string = newString;
		allocated = newAllocated;
	}

	string[length] = c;
	++length;
	string[length] = '\0';
}

void String::Reserve(SizeType reserveLength)
{
	if (reserveLength > allocated)
	{
		char* newString = new char[reserveLength + 1];
		std::memcpy(newString, string, length);
		newString[length] = '\0';

		delete[] string;
		string = newString;
		allocated = reserveLength;
	}
}

void String::Resize(SizeType size)
{
	if (size > length)
	{
		if (size > allocated)
		{
			char* newString = new char[size + 1];
			std::memcpy(newString, string, length);
			delete[] string;
			string = newString;
		}

		string[size] = '\0';
		allocated = size;
		length = size;
	}
	else if (size < length)
	{
		string[size] = '\0';
		length = size;
	}
}

String::SizeType String::CalculateAllocationSize(SizeType currentAllocated, SizeType requiredSize)
{
	SizeType newAllocated;
	
	if (currentAllocated > 1024)
		newAllocated = static_cast<SizeType>(currentAllocated * 1.5);
	else if (currentAllocated > 16)
		newAllocated = currentAllocated * 2;
	else
		newAllocated = 31;

	if (newAllocated < requiredSize)
		newAllocated = requiredSize;

	return newAllocated;
}

void String::Clear()
{
	delete[] string;

	string = nullptr;
	length = 0;
	allocated = 0;
}

inline String operator+(const String& lhs, const String& rhs)
{
	String result;
	String::SizeType leftLength = lhs.GetLength();
	String::SizeType rightLength = rhs.GetLength();
	String::SizeType combinedLength = leftLength + rightLength;
	
	if (combinedLength > 0)
	{
		result.Resize(combinedLength);
		std::memcpy(result.Begin(), lhs.Begin(), leftLength);
		std::memcpy(result.Begin() + leftLength, rhs.Begin(), rightLength);
	}

	return result;
}

inline String operator+(const String& lhs, StringRef rhs)
{
	String result;
	String::SizeType leftLength = lhs.GetLength();
	String::SizeType combinedLength = leftLength + rhs.len;

	if (combinedLength > 0)
	{
		result.Resize(combinedLength);
		std::memcpy(result.Begin(), lhs.Begin(), leftLength);
		std::memcpy(result.Begin() + leftLength, rhs.str, rhs.len);
	}

	return result;
}

inline String operator+(StringRef lhs, const String& rhs)
{
	// Call other operator function with reversed arguments
	return operator+(rhs, lhs);
}

