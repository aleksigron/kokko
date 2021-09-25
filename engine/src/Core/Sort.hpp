#pragma once

#include <cstdlib>

template <typename T>
void InsertionSortAsc(T* array, size_t count)
{
	for (size_t i = 1; i < count; ++i)
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
	for (size_t i = 1; i < count; ++i)
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
	for (size_t i = 1; i < count; ++i)
	{
		for (size_t k = i; k > 1 && pred(array[k], array[k - 1]); --k)
		{
			T temporary = array[k];
			array[k] = array[k - 1];
			array[k - 1] = temporary;
		}
	}
}

template <typename T>
void ShellSortPred(T* array, size_t count, bool(*pred)(const T&, const T&))
{
	// Sort an array a[0...n-1].
	size_t gaps[] = { 701, 301, 132, 57, 23, 10, 4, 1 };

	// Start with the largest gap and work down to a gap of 1
	for (size_t gap : gaps)
	{
		// Do a gapped insertion sort for this gap size.
		// The first gap elements array[0..gap-1] are already in gapped order
		// keep adding one more element until the entire array is gap sorted
		for (size_t i = gap; i < count; ++i)
		{
			// add array[i] to the elements that have been gap sorted
			// save array[i] in temp and make a hole at position i
			T temporary = array[i];
			size_t j = i;

			// shift earlier gap-sorted elements up until the correct location for array[i] is found
			while (j >= gap && pred(array[j - gap], temporary) == false)
			{
				array[j] = array[j - gap];
				j -= gap;
			}

			// put temp (the original array[i]) in its correct location
			array[j] = temporary;
		}
	}
}

template <typename T>
void ShellSortAsc(T* array, size_t count)
{
	// Sort an array a[0...n-1].
	size_t gaps[] = { 701, 301, 132, 57, 23, 10, 4, 1 };

	// Start with the largest gap and work down to a gap of 1
	for (size_t gap : gaps)
	{
		// Do a gapped insertion sort for this gap size.
		// The first gap elements array[0..gap-1] are already in gapped order
		// keep adding one more element until the entire array is gap sorted
		for (size_t i = gap; i < count; ++i)
		{
			// add array[i] to the elements that have been gap sorted
			// save array[i] in temp and make a hole at position i
			T temporary = array[i];
			size_t j = i;

			// shift earlier gap-sorted elements up until the correct location for array[i] is found
			while (j >= gap && (array[j - gap] < temporary) == false)
			{
				array[j] = array[j - gap];
				j -= gap;
			}

			// put temp (the original array[i]) in its correct location
			array[j] = temporary;
		}
	}
}

template <typename T>
void ShellSortDesc(T* array, size_t count)
{
	// Sort an array a[0...n-1].
	size_t gaps[] = { 701, 301, 132, 57, 23, 10, 4, 1 };

	// Start with the largest gap and work down to a gap of 1
	for (size_t gap : gaps)
	{
		// Do a gapped insertion sort for this gap size.
		// The first gap elements array[0..gap-1] are already in gapped order
		// keep adding one more element until the entire array is gap sorted
		for (size_t i = gap; i < count; ++i)
		{
			// add array[i] to the elements that have been gap sorted
			// save array[i] in temp and make a hole at position i
			T temporary = array[i];
			size_t j = i;

			// shift earlier gap-sorted elements up until the correct location for array[i] is found
			while (j >= gap && (array[j - gap] < temporary) == true)
			{
				array[j] = array[j - gap];
				j -= gap;
			}

			// put temp (the original array[i]) in its correct location
			array[j] = temporary;
		}
	}
}
