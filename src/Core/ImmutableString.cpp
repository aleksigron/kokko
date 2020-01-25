#include "Core/ImmutableString.hpp"

#include <cstring>

ImmutableString::ImmutableString()
{
}

ImmutableString::ImmutableString(const char* s)
{
	if (s != nullptr)
	{
		this->length = std::strlen(s);

		if (this->length > 0)
		{
			this->string = new char[this->length + 1];
			std::memcpy(this->string, s, this->length);
			this->string[this->length] = '\0';
		}
	}
}

ImmutableString::ImmutableString(const char* s, size_t len)
{
	if (s != nullptr && len > 0)
	{
		this->length = len;
		this->string = new char[this->length + 1];
		std::memcpy(this->string, s, len);
		this->string[this->length] = '\0';
	}
}

ImmutableString::ImmutableString(const ImmutableString& other)
{
	if (other.string != nullptr)
	{
		this->length = other.length;
		this->string = new char[this->length + 1];
		std::memcpy(this->string, other.string, this->length);
		this->string[this->length] = '\0';
	}
}

ImmutableString::ImmutableString(ImmutableString&& other)
{
	this->length = other.length;
	this->string = other.string;

	other.length = 0;
	other.string = nullptr;
}

ImmutableString::~ImmutableString()
{
	delete[] this->string;
}

ImmutableString& ImmutableString::operator=(const ImmutableString& other)
{
	if (other.string != nullptr && other.length > 0)
	{
		delete[] this->string;

		this->length = other.length;
		this->string = new char[this->length + 1];

		std::memcpy(this->string, other.string, other.length);
		this->string[this->length] = '\0';
	}

	return *this;
}

ImmutableString& ImmutableString::operator=(ImmutableString && other)
{
	delete[] this->string;

	this->length = other.length;
	this->string = other.string;

	other.length = 0;
	other.string = nullptr;

	return *this;
}
