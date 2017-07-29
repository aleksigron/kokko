#pragma once

struct StringRef
{
	const char* str;
	unsigned int len;

	// Create a empty StringRef instance
	StringRef() : str(nullptr), len(0) {}

	// Create a StringRef instance from char pointer and length
	StringRef(const char* string, unsigned int length) : str(string), len(length) {}

	// Create a StringRef instance from a c-string
	explicit StringRef(const char* string) : str(string)
	{
		while (*string != '\0')
		{
			++string;
		}

		len = static_cast<unsigned int>(string - str);
	}

	// Does the other StringRef object reference the same area in memory
	bool ReferenceEquals(const StringRef& other) const
	{
		return this->str == other.str && this->len == other.len;
	}

	// Does the other StringRef object contain the same value
	bool ValueEquals(const StringRef& other) const;

	// Does the c-string contain the same value as this StringRef
	bool ValueEquals(const char* cstring) const;

	bool operator == (const StringRef& other) const
	{
		return this->ValueEquals(other);
	}

	bool operator != (const StringRef& other) const
	{
		return this->ValueEquals(other) == false;
	}

	// Return true if this->str is not null
	bool IsValid() const { return str != nullptr; }

	// Clear this object
	void Invalidate()
	{
		str = nullptr;
		len = 0;
	}

	// Modify the reference to remove the specified amount of bytes from the beginning
	void TrimBeginning(unsigned int amount);

	// Modify the reference to remove the specified amount of bytes from the end
	void TrimEnd(unsigned int amount);
};
