#include "Core/SortedArray.hpp"

#include "doctest/doctest.h"

#include "Memory/Allocator.hpp"

namespace kokko
{

TEST_CASE("SortedArray sorts inserted items")
{
	SortedArray<int> a(Allocator::GetDefault());

	CHECK(a.GetCount() == 0);
	a.Insert(2);
	CHECK(a.GetCount() == 1);
	CHECK(a[0] == 2);
	a.Insert(1);
	CHECK(a[0] == 1);
	CHECK(a[1] == 2);
}

TEST_CASE("SortedArray InsertUnique only allows unique items")
{
	SortedArray<int> a(Allocator::GetDefault());

	a.InsertUnique(2);
	CHECK(a.GetCount() == 1);

	a.InsertUnique(1);
	CHECK(a.GetCount() == 2);

	a.InsertUnique(0);
	CHECK(a.GetCount() == 3);

	a.InsertUnique(1);
	CHECK(a.GetCount() == 3);
}

TEST_CASE("SortedArray Contains correctly finds items")
{
	SortedArray<int> a(Allocator::GetDefault());

	a.Insert(2);
	a.Insert(1);

	CHECK(a.Contains(2) == true);
	CHECK(a.Contains(1) == true);
	CHECK(a.Contains(0) == false);

	a.Clear();
	a.Insert(1);
	CHECK(a.Contains(2) == false);
}

TEST_CASE("SortedArray Find correctly finds items")
{
	SortedArray<int> a(Allocator::GetDefault());

	a.Insert(0);
	a.Insert(1);
	a.Insert(2);

	CHECK(a.Find(0) == 0);
	CHECK(a.Find(1) == 1);
	CHECK(a.Find(2) == 2);
	CHECK(a.Find(3) < 0);

	a.Insert(0);
	CHECK(a.Find(0) >= 0);

	a.Insert(2);
	CHECK(a.Find(2) >= 0);
}

} // namespace kokko
