#include "Core/String.hpp"

#include <cassert>
#include <cstring>

#include "doctest/doctest.h"

#include "Core/CString.hpp"
#include "Core/Hash.hpp"
#include "Core/StringRef.hpp"

#include "Memory/Allocator.hpp"

doctest::String toString(const kokko::String& value)
{
	return doctest::String(value.GetCStr(), static_cast<unsigned int>(value.GetLength()));
}

namespace kokko
{

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
		size_t bufferSize = allocated + 1;
		string = static_cast<char*>(allocator->Allocate(bufferSize));
		
		size_t len = s.GetLength();
		std::memcpy(string, s.GetCStr(), len);
		string[len] = '\0';
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
		size_t bufferSize = allocated + 1;
		string = static_cast<char*>(allocator->Allocate(bufferSize));
		StringCopyN(string, s, bufferSize);
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
	size_t newLength = s.GetLength();

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

StringRef String::GetRef() const
{
	return StringRef(string, length);
}

void String::Append(StringRef s)
{
	if (s.len != 0)
	{
		size_t requiredLength = length + s.len;

		if (allocated < requiredLength)
		{
			size_t newAllocated = CalculateAllocationSize(allocated, requiredLength);

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
		size_t newAllocated = CalculateAllocationSize(this->allocated, length + 1);

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

void String::Assign(StringRef s)
{
	Clear();
	Append(s);
}

void String::Assign(const String& s)
{
	Clear();
	Append(s);
}

void String::Assign(const char* s)
{
	Clear();
	Append(s);
}

void String::Reserve(size_t reserveLength)
{
	if (reserveLength > allocated)
	{
		size_t bufferSize = reserveLength + 1;
		char* newString = static_cast<char*>(allocator->Allocate(bufferSize));

		if (string != nullptr)
		{
			StringCopyN(newString, string, bufferSize);

			allocator->Deallocate(string);
		}

		string = newString;
		allocated = reserveLength;
	}
}

void String::Resize(size_t size)
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

size_t String::CalculateAllocationSize(size_t currentAllocated, size_t requiredSize)
{
	size_t newAllocated;
	
	if (currentAllocated > 1024)
		newAllocated = static_cast<size_t>(currentAllocated * 1.5);
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
	size_t leftLength = lhs.GetLength();
	size_t combinedLength = leftLength + rhs.len;

	if (combinedLength > 0)
	{
		result.Resize(combinedLength);
		std::memcpy(result.Begin(), lhs.Begin(), leftLength);
		std::memcpy(result.Begin() + leftLength, rhs.str, rhs.len);
	}

	return result;
}

TEST_CASE("String.operator+(const String&, StringRef)")
{
	Allocator* a = Allocator::GetDefault();

	CHECK((String(a) + StringRef("")).GetLength() == 0);
	CHECK((String(a, "") + StringRef("")).GetLength() == 0);
	CHECK(String(a, "Test") + StringRef("") == String(a, "Test"));
	CHECK(String(a) + StringRef("Test") == String(a, "Test"));
	CHECK(String(a, "Test ") + StringRef("string") == String(a, "Test string"));
}

String operator+(StringRef lhs, const String& rhs)
{
	String result(rhs.allocator);
	size_t leftLength = lhs.len;
	size_t rightLength = rhs.GetLength();
	size_t combinedLength = leftLength + rightLength;

	if (combinedLength > 0)
	{
		result.Resize(combinedLength);
		std::memcpy(result.Begin(), lhs.str, leftLength);
		std::memcpy(result.Begin() + leftLength, rhs.GetData(), rightLength);
	}

	return result;
}

TEST_CASE("String.operator+(StringRef, const String&)")
{
	Allocator* a = Allocator::GetDefault();

	CHECK((StringRef("") + String(a)).GetLength() == 0);
	CHECK((StringRef("") + String(a, "")).GetLength() == 0);
	CHECK(StringRef("") + String(a, "Test") == String(a, "Test"));
	CHECK(StringRef("Test") + String(a) == String(a, "Test"));
	CHECK(StringRef("Test ") + String(a, "string") == String(a, "Test string"));
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
	return operator+(StringRef(lhs), rhs);
}

String operator+(const String& lhs, const char rhs)
{
	return operator+(lhs, StringRef(&rhs, 1));
}

String operator+(char lhs, const String& rhs)
{
	return operator+(StringRef(&lhs, 1), rhs);
}

bool operator==(const String& lhs, const String& rhs)
{
	return operator==(lhs.GetRef(), rhs);
}

bool operator==(const String& lhs, const char* rhs)
{
	return lhs == StringRef(rhs);
}

bool operator==(const char* lhs, const String& rhs)
{
	return operator==(rhs, lhs); // Swap arguments
}

bool operator==(const String& lhs, StringRef rhs)
{
	return operator==(rhs, lhs);
}

bool operator==(StringRef lhs, const String& rhs)
{
	size_t length = rhs.GetLength();

	if (lhs.len != length)
		return false;

	for (size_t i = 0; i < length; ++i)
		if (lhs[i] != rhs[i])
			return false;

	return true;
}

TEST_CASE("String.operator==")
{
	Allocator* a = Allocator::GetDefault();

	CHECK(String() == String());
	CHECK(String(a) == String(a));
	CHECK(String(a, "") == String(a, ""));
	CHECK(String(a, "") == String());

	CHECK((String(a, "T") == String(a, "T")) == true);
	CHECK((String(a, "T") == String(a, "")) == false);

	CHECK((String(a, "Test") == String(a, "Test")) == true);
	CHECK((String(a, "Test") == String(a, "Tes ")) == false);
	CHECK((String(a, "Test") == String(a, "Tes")) == false);
	CHECK((String(a, "Test") == String(a, "Tests")) == false);

	String testStr(a, "Test string");
	CHECK((testStr == StringRef("Test string")) == true);
	CHECK((testStr == StringRef("")) == false);
	CHECK((testStr == StringRef("Test strin")) == false);
	CHECK((testStr == StringRef("Test strings")) == false);

	String emptyStr(a);
	CHECK((emptyStr == "") == true);
	CHECK((emptyStr == "T") == false);
	CHECK((emptyStr == "Test string") == false);
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

bool operator!=(const String& lhs, StringRef rhs)
{
	return operator==(lhs, rhs) == false;
}

bool operator!=(StringRef lhs, const String& rhs)
{
	return operator==(lhs, rhs) == false;
}

uint32_t Hash32(const String& value, uint32_t seed)
{
	return Hash32(value.GetData(), value.GetLength(), seed);
}

}
