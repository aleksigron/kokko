#include "Core/StringRef.hpp"

bool StringRef::ValueEquals(const StringRef& other) const
{
	if (len != other.len || str == nullptr || other.str == nullptr)
		return false;

	for (unsigned i = 0; i < len; ++i)
		if (str[i] != other.str[i])
			return false;

	return true;
}

bool StringRef::ValueEquals(const char* cstring) const
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

void StringRef::TrimBeginning(unsigned int amount)
{
	if (amount < len)
	{
		str = str + amount;
		len = len - amount;
	}
	else
		this->Clear();
}

void StringRef::TrimEnd(unsigned int amount)
{
	if (amount < len)
		len = len - amount;
	else
		this->Clear();
}
