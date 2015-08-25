#pragma once

struct StringRef
{
	const char* str = nullptr;
	unsigned int len = 0;

	inline bool ReferenceEquals(const StringRef& other) const
	{ return this->str == other.str && this->len == other.len; }

	bool ValueEquals(const StringRef& other) const
	{
		if (len != other.len || str == nullptr || other.str == nullptr)
			return false;

		for (unsigned i = 0; i < len; ++i)
			if (str[i] != other.str[i])
				return false;

		return true;
	}

	bool ValueEquals(const char* cstring) const
	{
		if (this->str == nullptr || cstring == nullptr)
			return false;

		unsigned int i = 0;
		while (i < this->len)
		{
			if (cstring[i] == '\0' || this->str[i] != cstring[i])
				return false;

			++i;
		}

		// The strings match for the length of this StringRef
		// If cstring too ends here, the strings are equal
		return cstring[i] == '\0';
	}

	inline bool operator == (const StringRef& other) const
	{ return this->ValueEquals(other); }

	inline bool operator != (const StringRef& other) const
	{ return this->ValueEquals(other) == false; }

	inline bool IsValid() const
	{ return str != nullptr; }

	inline operator bool() const
	{ return this->IsValid(); }

	inline void Invalidate()
	{
		str = nullptr;
		len = 0;
	}
};