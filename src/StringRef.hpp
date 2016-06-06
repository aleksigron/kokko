#pragma once

struct StringRef
{
	const char* str;
	unsigned int len;

	StringRef() : str(nullptr), len(0) {}

	StringRef(const char* string, unsigned int length) : str(string), len(length) {}

	explicit StringRef(const char* string) : str(string)
	{
		while (*string != '\0')
		{
			++string;
		}

		len = static_cast<unsigned int>(string - str);
	}

	bool ReferenceEquals(const StringRef& other) const
	{
		return this->str == other.str && this->len == other.len;
	}

	bool ValueEquals(const StringRef& other) const;
	bool ValueEquals(const char* cstring) const;

	bool operator == (const StringRef& other) const
	{
		return this->ValueEquals(other);
	}

	bool operator != (const StringRef& other) const
	{
		return this->ValueEquals(other) == false;
	}

	bool IsValid() const { return str != nullptr; }

	void Invalidate()
	{
		str = nullptr;
		len = 0;
	}

	void TrimBeginning(unsigned int amount);
	void TrimEnd(unsigned int amount);
};
