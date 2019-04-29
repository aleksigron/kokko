#pragma once

struct BitPack
{
	using DataType = unsigned int;

	constexpr static unsigned int BitsPerPack = sizeof(DataType) * 8;

	constexpr static unsigned int ValueMask = 0x1;
	constexpr static unsigned int CellIndexMask = BitsPerPack - 1;
	constexpr static unsigned int PackIndexMask = ~ CellIndexMask;

	constexpr static unsigned int PackIndex(unsigned int index) { return index & PackIndexMask; }
	constexpr static unsigned int CellIndex(unsigned int index) { return index & CellIndexMask; }

	/**
	 * Calculate the required number of BitPack instances to hold <valueCount>
	 * bits.
	 */
	constexpr static unsigned int CalculateRequired(unsigned int valueCount)
	{
		return (valueCount + BitsPerPack - 1) / BitsPerPack;
	}

	/**
	 * Set a single value in an array of BitPack
	 */
	static void Set(BitPack* packs, unsigned int index, bool v)
	{
		packs[PackIndex(index)].Set(CellIndex(index), v);
	}
	
	/**
	 * Get a single value from an array of BitPack
	 */
	static bool Get(BitPack* packs, unsigned int index)
	{
		return packs[PackIndex(index)].Get(CellIndex(index));
	}

	DataType data;

	/**
	 * Set a single value in this instance
	 */
	void Set(unsigned int index, bool v)
	{
		data = data & (~(ValueMask << index)); // Set bit to 0
		data = data | ((v ? 1 : 0) << index); // Or bit with new value
	}

	/**
	 * Set an array of BitsPerPack values in this BitPack
	 */
	void Set(bool* states)
	{
		data = 0;

		for (unsigned int i = 0; i < BitsPerPack; ++i)
			data |= (states[i] ? 1 : 0) << i;
	}

	/**
	 * Get a single value from this instance
	 */
	bool Get(unsigned int index)
	{
		return static_cast<bool>((data >> index) & ValueMask);
	}
};
