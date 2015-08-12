#pragma once

struct StringRef
{
	const char* str = nullptr;
	unsigned int len = 0;

	bool operator == (const StringRef& other) const
	{
		if (len != other.len)
			return false;
		for (unsigned i = 0; i < len; ++i)
			if (str[i] != other.str[i])
				return false;
		return true;
	}

	inline bool operator != (const StringRef& other) const
	{ return operator == (other) == false; }

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