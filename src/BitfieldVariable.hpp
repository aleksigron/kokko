#pragma once

template <typename T>
struct BitfieldVariable
{
	T bits, mask, shift;

	BitfieldVariable() : bits(0), mask(0), shift(0)
	{
	}

	void SetDefinition(T bits, T unusedBits)
	{
		this->bits = bits;
		this->mask = static_cast<T>((1LL << bits) - 1);
		this->shift = unusedBits - bits;
	}

	void AssignValue(T& bitfield, T value) const
	{
		value = value & this->mask;
		value = value << this->shift;
		bitfield = bitfield | value;
	}

	void AssignZero(T& bitfield) const
	{
		T inverseMask = ~(this->mask);
		bitfield = bitfield & inverseMask;
	}
};
