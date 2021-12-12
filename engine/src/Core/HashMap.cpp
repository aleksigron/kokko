#include "Core/HashMap.hpp"

#include "doctest/doctest.h"

#include <string>

namespace kokko
{
static uint32_t Hash32(const std::string& value, uint32_t seed)
{
	return kokko::Hash32(value.c_str(), value.length(), seed);
}
}

TEST_CASE("HashMap.Lookup")
{
	Allocator* allocator = Allocator::GetDefault();
	HashMap<int, int> map(allocator);

	constexpr int count = 16;
	for (int i = 0; i < count; ++i)
	{
		map.Insert(i * 4)->second = i;
	}

	for (int i = 0; i < count; ++i)
	{
		auto pair = map.Lookup(i * 4);
		CHECK(pair != nullptr);
		CHECK(pair->second == i);
	}
}

TEST_CASE("HashMap.LookupNonTrivial")
{
	Allocator* allocator = Allocator::GetDefault();
	HashMap<std::string, std::string> map(allocator);

	auto getString = [](int i) { return std::to_string(i); };

	constexpr int count = 16;
	for (int i = 0; i < count; ++i)
	{
		std::string s = getString(i);
		auto* pair = map.Insert(s);
		pair->second = s;
	}

	for (int i = 0; i < count; ++i)
	{
		std::string s = getString(i);
		auto pair = map.Lookup(s);
		CHECK(pair != nullptr);
		CHECK(pair->second == s);
	}
}

TEST_CASE("HashMap.Iterator")
{
	Allocator* allocator = Allocator::GetDefault();
	HashMap<int, int> map(allocator);

	constexpr int count = 16;
	bool intFound[count] = { false };
	bool nonExistantFound = false;
	
	for (int i = 0; i < count; ++i)
	{
		map.Insert(i)->second = i;
	}

	for (auto pair : map)
	{
		int key = pair.first;
		if (key < 0 || key >= count)
		{
			nonExistantFound = true;
		}
		else
		{
			intFound[key] = true;
			CHECK(pair.second == key);
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
