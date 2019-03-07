#pragma once

enum class CullState : unsigned char
{
	Outside = 0,
	Intersect = 1,
	Inside = 2
};

struct CullStatePacked16
{
	// Static members

	constexpr static unsigned int StateBits = 2;
	constexpr static unsigned int Count = 16;

	constexpr static unsigned int CellIndexMask = Count - 1;
	constexpr static unsigned int PackIndexMask = ~ CellIndexMask;

	constexpr static unsigned int PackIndex(unsigned int index) { return index & PackIndexMask; }
	constexpr static unsigned int CellIndex(unsigned int index) { return index & CellIndexMask; }

	/**
	 * Calculate the required number of CullStatePacked16 instances to hold <stateCount>
	 * CullState instances.
	 */
	constexpr static unsigned int CalculateRequired(unsigned int stateCount)
	{
		return (stateCount + Count - 1) / Count;
	}

	/**
	 * Set a single CullState in an array of CullStatePacked16
	 */
	static void Set(CullStatePacked16* states, unsigned int index, CullState s)
	{
		states[PackIndex(index)].Set(CellIndex(index), s);
	}

	/**
	 * Set a single CullState in an array of CullStatePacked16
	 */
	static bool IsOutside(CullStatePacked16* states, unsigned int index)
	{
		return states[PackIndex(index)].IsOutside(CellIndex(index));
	}

	// Instance members

	unsigned int data;


	/**
	 * Set a single CullState in this instance of CullStatePacked16
	 */
	void Set(unsigned int index, CullState state)
	{
		data &= ~(0b11 << (StateBits * index));
		data |= static_cast<unsigned int>(state) << (StateBits * index);
	}

	/**
	 * Set an array of 16 CullState instances in this CullStatePacked16
	 */
	void Set(CullState* states)
	{
		data = 0;

		for (unsigned int i = 0; i < Count; ++i)
			data |= static_cast<unsigned int>(states[i]) << (StateBits * i);
	}

	/**
	 * Get a single CullState from this instance
	 */
	CullState Get(unsigned int index)
	{
		return static_cast<CullState>((data >> (StateBits * index)) & 0b11);
	}

	/**
	 * Get whether a single CullState from this instance is CullState::Outside
	 */
	bool IsOutside(unsigned int index)
	{
		return ((data >> (StateBits * index)) & 0b11) == 0;
	}
};
