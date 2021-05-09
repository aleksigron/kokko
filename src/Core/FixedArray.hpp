#pragma once

template <typename ValueType, unsigned int Capacity>
class FixedArray
{
public:
	constexpr unsigned int GetCapacity() const { return Capacity; }

	ValueType* GetData() { return array; }
	const ValueType* GetData() const { return array; }

	ValueType& At(unsigned int index) { return array[index]; }
	const ValueType& At(unsigned int index) const { return array[index]; }

	ValueType& operator[](unsigned int index) { return array[index]; }
	const ValueType& operator[](unsigned int index) const { return array[index]; }

private:
	ValueType array[Capacity];
};
