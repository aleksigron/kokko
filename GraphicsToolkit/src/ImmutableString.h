#pragma once

#include <cstddef>
#include <cstring>

class ImmutableString
{
private:
	char* string = nullptr;
	size_t size = 0;

public:
	ImmutableString(const char* s)
	{
		if (s != nullptr)
		{
			this->size = std::strlen(s);

			if (size > 0)
			{
				this->string = new char[this->size + 1];
				std::strcpy(this->string, s);
			}
		}
	}

	ImmutableString(const char* s, size_t len)
	{
		if (s != nullptr && len > 0)
		{
			this->size = len;
			this->string = new char[this->size + 1];
			std::memcpy(this->string, s, len);
			this->string[this->size] = '\0';
		}
	}

	ImmutableString(const ImmutableString& other)
	{
		if (other.string != nullptr)
		{
			this->size = other.size;
			this->string = new char[this->size + 1];
			std::strcpy(this->string, other.string);
		}
	}

	~ImmutableString()
	{
		if (this->string != nullptr)
			delete[] this->string;
	}

	inline const char* c_string() const
	{
		return this->string;
	}

	inline size_t length() const
	{
		return this->size;
	}
};