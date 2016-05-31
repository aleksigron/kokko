#pragma once

#include <cstdlib>

template <typename T>
void InsertionSortAsc(T* array, size_t count)
{
	for (size_t i = 2; i < count; ++i)
	{
		for (size_t k = i; k > 1 && array[k] < array[k - 1]; --k)
		{
			T temporary = array[k];
			array[k] = array[k - 1];
			array[k - 1] = temporary;
		}
	}
}

template <typename T>
void InsertionSortDesc(T* array, size_t count)
{
	for (size_t i = 2; i < count; ++i)
	{
		for (size_t k = i; k > 1 && array[k] > array[k - 1]; --k)
		{
			T temporary = array[k];
			array[k] = array[k - 1];
			array[k - 1] = temporary;
		}
	}
}

template <typename T>
void InsertionSortPred(T* array, size_t count, bool(*pred)(const T&, const T&))
{
	for (size_t i = 2; i < count; ++i)
	{
		for (size_t k = i; k > 1 && pred(array[k], array[k - 1]); --k)
		{
			T temporary = array[k];
			array[k] = array[k - 1];
			array[k - 1] = temporary;
		}
	}
}
