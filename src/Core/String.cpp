#include "Core/String.hpp"

#include <cassert>
#include <cstring>

#include "Memory/Allocator.hpp"

String::String() :
	allocator(nullptr),
	string(nullptr),
	length(0),
	allocated(0)
{
}

String::String(Allocator* allocator) :
	allocator(allocator),
	string(nullptr),
	length(0),
	allocated(0)
{
}

String::String(const String& s)
{
	allocator = s.allocator;

	length = s.GetLength();
	allocated = CalculateAllocationSize(0, length);

	if (allocated > 0)
	{
		string = static_cast<char*>(allocator->Allocate(allocated + 1));
		std::strcpy(string, s.GetCStr());
	}
	else
		string = nullptr;
}

String::String(String&& s) noexcept
{
	allocator = s.allocator;
	string = s.string;
	length = s.length;
	allocated = s.allocated;

	s.string = nullptr;
	s.length = 0;
	s.allocated = 0;
}

String::String(Allocator* allocator, const char* s) :
	allocator(allocator)
{
	length = std::strlen(s);
	allocated = CalculateAllocationSize(0, length);

	if (allocated > 0)
	{
		string = static_cast<char*>(allocator->Allocate(allocated + 1));
		std::strcpy(string, s);
	}
	else
		string = nullptr;
}

String::String(Allocator* allocator, StringRef s) :
	allocator(allocator)
{
	length = s.len;
	allocated = CalculateAllocationSize(0, length);

	if (allocated > 0)
	{
		string = static_cast<char*>(allocator->Allocate(allocated + 1));
		std::memcpy(string, s.str, length);
		string[length] = '\0';
	}
	else
		string = nullptr;
}

String::~String()
{
	if (allocator != nullptr)
		allocator->Deallocate(string);
}

String& String::operator=(const String& s)
{
	SizeType newLength = s.GetLength();

	if (newLength > allocated)
	{
		if (allocator != nullptr)
			allocator->Deallocate(string);

		allocator = s.allocator;

		allocated = CalculateAllocationSize(allocated, newLength);
		string = static_cast<char*>(allocator->Allocate(allocated + 1));
	}
	else
		allocator = s.allocator;

	if (newLength > 0)
	{
		std::memcpy(string, s.GetCStr(), newLength);
		string[newLength] = '\0';
	}

	length = newLength;

	return *this;
}

String& String::operator=(String&& s) noexcept
{
	if (allocator != nullptr)
		allocator->Deallocate(string);

	allocator = s.allocator;
	string = s.string;
	length = s.length;
	allocated = s.allocated;

	s.string = nullptr;
	s.length = 0;
	s.allocated = 0;

	return *this;
}

void String::SetAllocator(Allocator* allocator)
{
	assert(this->allocator == nullptr);

	this->allocator = allocator;
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

		char* newString = static_cast<char*>(allocator->Allocate(newAllocated + 1));
		std::memcpy(newString, string, length);
		allocator->Deallocate(string);

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

		char* newString = static_cast<char*>(allocator->Allocate(newAllocated + 1));
		std::memcpy(newString, string, length);
		allocator->Deallocate(string);

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
		char* newString = static_cast<char*>(allocator->Allocate(reserveLength + 1));

		if (string != nullptr)
		{
			std::strcpy(newString, string);

			allocator->Deallocate(string);
		}

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
			char* newString = static_cast<char*>(allocator->Allocate(size + 1));

			if (this->string != nullptr)
			{
				std::memcpy(newString, string, length);
				allocator->Deallocate(string);
			}

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
	length = 0;

	if (string != nullptr)
	{
		string[length] = '\0';
	}
}

String operator+(const String& lhs, StringRef rhs)
{
	String result(lhs.allocator);
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

String operator+(StringRef lhs, const String& rhs)
{
	// Call with reversed arguments
	return operator+(rhs, lhs);
}

String operator+(const String& lhs, const String& rhs)
{
	return operator+(lhs, rhs.GetRef());
}

String operator+(const String& lhs, const char* rhs)
{
	return operator+(lhs, StringRef(rhs));
}

String operator+(const char* lhs, const String& rhs)
{
	// Call with reversed arguments
	return operator+(rhs, StringRef(lhs));
}

bool operator==(const String& lhs, const String& rhs)
{
	if (lhs.GetLength() == rhs.GetLength())
		for (unsigned int i = 0, len = lhs.GetLength(); i < len; ++i)
			if (lhs[i] != rhs[i])
				return false;

	return true;
}

bool operator==(const String& lhs, const char* rhs)
{
	for (unsigned int i = 0, len = lhs.GetLength(); i < len; ++i)
		if (lhs[i] != rhs[i] || lhs[i] == '\0')
			return false;

	return true;
}

bool operator==(const char* lhs, const String& rhs)
{
	return operator==(rhs, lhs); // Swap arguments
}

bool operator!=(const String& lhs, const String& rhs)
{
	return operator==(lhs, rhs) == false;
}

bool operator!=(const String& lhs, const char* rhs)
{
	return operator==(lhs, rhs) == false;
}

bool operator!=(const char* lhs, const String& rhs)
{
	return operator==(lhs, rhs) == false;
}
