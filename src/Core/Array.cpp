#include "Array.hpp"

#include "doctest/doctest.h"

#include "Memory/MetricAllocator.hpp"

TEST_CASE("Array can hold items")
{
	Array<int> a(Allocator::GetDefault());

	CHECK(a.GetCount() == 0);
	a.PushBack(1);
	CHECK(a.GetCount() == 1);
	CHECK(a[0] == 1);
	a.PushBack(2);
	CHECK(a[0] == 1);
	CHECK(a[1] == 2);
}

TEST_CASE("Array multiple items can be inserted")
{
	Array<int> a(Allocator::GetDefault());

	int values[] = { 0, 1, 2 };
	a.Insert(0, values, 3);

	CHECK(a.GetCount() == 3);
	CHECK(a[0] == 0);
	CHECK(a[1] == 1);
	CHECK(a[2] == 2);
}

TEST_CASE("Array items can be removed")
{
	Array<int> a(Allocator::GetDefault());

	int values[] = { 0, 1, 2 };
	a.Insert(0, values, 3);

	a.Remove(2);
	CHECK(a.GetCount() == 2);
	CHECK(a.GetBack() == 1);
	
	a.Remove(0);
	CHECK(a.GetCount() == 1);
	CHECK(a[0] == 1);
}

TEST_CASE("Array can be iterated")
{
	Array<int> a(Allocator::GetDefault());

	int values[] = { 0, 1, 2 };
	a.Insert(0, values, 3);

	CHECK(a.GetCount() == 3);

	{
		int iterationCount = 0;
		Array<int>::Iterator itr = a.begin();
		Array<int>::Iterator end = a.end();
		for (; itr != end; ++itr)
		{
			CHECK(*itr == iterationCount);
			iterationCount += 1;
		}
	}

	{
		int iterationCount = 0;
		for (int val : a)
		{
			CHECK(val == iterationCount);
			iterationCount += 1;
		}
	}
}

TEST_CASE("Array memory is allocated and released")
{
	Allocator* allocator = Allocator::GetDefault();
	MetricAllocator metricAllocator(nullptr, allocator);
	{
		Array<int> a(&metricAllocator);

		int values[] = { 0, 1, 2 };
		a.Insert(0, values, 3);

		CHECK(metricAllocator.GetTotalAllocationCount() == 1);
		CHECK(metricAllocator.GetTotalAllocationSize() > 0);
	}

	CHECK(metricAllocator.GetTotalAllocationCount() == 0);
	CHECK(metricAllocator.GetTotalAllocationSize() == 0);
}
