#include "Core/HashMap.hpp"

#include "doctest/doctest.h"

TEST_CASE("HashMap.Iterator")
{
	Allocator* allocator = Allocator::GetDefault();
	HashMap<int, int> map(allocator);

	constexpr int count = 4;
	bool intFound[count] = { false };
	bool nonExistantFound = false;
	
	for (int i = 0; i < count; ++i)
	{
		map.Insert(i)->second = i;
	}

	for (auto itr = map.begin(), end = map.end(); itr != end; ++itr)
	{
		int key = itr->first;
		if (key < 0 || key >= count)
		{
			nonExistantFound = true;
		}
		else
		{
			intFound[key] = true;
			CHECK(itr->second == key);
		}
	}

	CHECK(nonExistantFound == false);

	for (int i = 0; i < count; ++i)
	{
		CHECK(intFound[i] == true);
	}
}

TEST_CASE("HashMap.IteratorEmpty")
{
	Allocator* allocator = Allocator::GetDefault();
	HashMap<int, int> map(allocator);

	int count = 0;
	for (auto pair : map)
	{
		count += 1;
	}

	CHECK(count == 0);

	map.Insert(0)->second = 0;
	map.Insert(1)->second = 1;
	map.Clear();

	for (auto pair : map)
	{
		count += 1;
	}

	CHECK(count == 0);
}
