#pragma once

#include <cassert>

namespace kokko
{

template <typename T>
struct BitfieldVariable
{
	T bits, mask, shift;

	BitfieldVariable() : bits(0), mask(0), shift(0)
	{
	}

	void SetDefinition(T bits, T unusedBits)
	{
		assert(bits <= unusedBits);

		this->bits = bits;
		this->mask = static_cast<T>((1ULL << bits) - 1);
		this->shift = unusedBits - bits;
	}

	void AssignValue(T& bitfield, T value) const
	{
		T bitfieldValue = value & this->mask;
		assert(bitfieldValue == value);
		bitfieldValue = bitfieldValue << this->shift;
		bitfield = bitfield | bitfieldValue;
	}

	void AssignZero(T& bitfield) const
	{
		T inverseMask = ~(this->mask << this->shift);
		bitfield = bitfield & inverseMask;
	}

	T GetValue(T bitfield) const
	{
		T value = bitfield >> this->shift;
		value = value & this->mask;
		return value;
	}
};

} // namespace kokko
