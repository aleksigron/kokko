#pragma once

enum class CullState : unsigned char
{
	Outside = 0,
	Intersect = 1,
	Inside = 2
};

struct CullStatePacked16
{
	constexpr static unsigned int StateBits = 2;
	constexpr static unsigned int Count = 16;

	constexpr static unsigned int CellIndexMask = Count - 1;
	constexpr static unsigned int PackIndexMask = ~ CellIndexMask;

	static unsigned int PackIndex(unsigned int index) { return index & PackIndexMask; }
	static unsigned int CellIndex(unsigned int index) { return index & CellIndexMask; }

	static unsigned int CalculateRequired(unsigned int stateCount)
	{
		return (stateCount + Count - 1) / Count;
	}

	unsigned int data;

	void Set(unsigned int index, CullState state)
	{
		data &= ~(0b11 << (StateBits * index));
		data |= static_cast<unsigned int>(state) << (StateBits * index);
	}

	void Set(CullState* states)
	{
		data = 0;

		for (unsigned int i = 0; i < Count; ++i)
			data |= static_cast<unsigned int>(states[i]) << (StateBits * i);
	}

	CullState Get(unsigned int index)
	{
		return static_cast<CullState>((data >> (StateBits * index)) & 0b11);
	}

	bool IsOutside(unsigned int index)
	{
		return ((data >> (StateBits * index)) & 0b11) == 0;
	}
};
