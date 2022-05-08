#include "Core/HashMap.hpp"

#include "doctest/doctest.h"

#include "Core/String.hpp"

TEST_CASE("HashMap.LookupAndModify")
{
	Allocator* allocator = Allocator::GetDefault();
	HashMap<int, int> map(allocator);

	auto key = [](int i) { return (i + 31) * 7; };

	constexpr int count = 16;
	for (int i = 0; i < count; ++i)
	{
		map.Insert(key(i))->second = i;
	}

	// Check const lookup

	const HashMap<int, int>& constMap = map;

	for (int i = 0; i < count; ++i)
	{
		auto pair = constMap.Lookup(key(i));
		CHECK(pair != nullptr);
		CHECK(pair->second == i);
	}

	// Check non-const lookup and modify

	for (int i = 0; i < count; ++i)
	{
		auto pair = map.Lookup(key(i));
		CHECK(pair != nullptr);
		CHECK(pair->second == i);

		pair->second = i * 5;
	}

	// Check modified value

	for (int i = 0; i < count; ++i)
	{
		auto pair = map.Lookup(key(i));
		CHECK(pair != nullptr);
		CHECK(pair->second == i * 5);
	}
}

TEST_CASE("HashMap.CopyAndMove")
{
	Allocator* allocator = Allocator::GetDefault();
	HashMap<int, int> map(allocator);

	auto key = [](int i) { return (i + 31) * 7; };

	constexpr int count = 16;
	for (int i = 0; i < count; ++i)
	{
		map.Insert(key(i))->second = i;
	}

	HashMap<int, int> copyConstruct(map);

	for (int i = 0; i < count; ++i)
	{
		auto pair = copyConstruct.Lookup(key(i));
		CHECK(pair != nullptr);
		CHECK(pair->second == i);
	}

	HashMap<int, int> copyAssign(allocator);
	copyAssign = map;

	for (int i = 0; i < count; ++i)
	{
		auto pair = copyAssign.Lookup(key(i));
		CHECK(pair != nullptr);
		CHECK(pair->second == i);
	}

	HashMap<int, int> moveConstruct(std::move(map));
	for (int i = 0; i < count; ++i)
	{
		CHECK(map.Lookup(key(i)) == nullptr);

		auto pair = moveConstruct.Lookup(key(i));
		CHECK(pair != nullptr);
		CHECK(pair->second == i);
	}

	HashMap<int, int> moveAssign(allocator);
	moveAssign = std::move(copyConstruct);

	for (int i = 0; i < count; ++i)
	{
		CHECK(copyConstruct.Lookup(key(i)) == nullptr);

		auto pair = moveAssign.Lookup(key(i));
		CHECK(pair != nullptr);
		CHECK(pair->second == i);
	}
}

TEST_CASE("HashMap.LookupNonTrivial")
{
	Allocator* allocator = Allocator::GetDefault();
	HashMap<kokko::String, kokko::String> map(allocator);

	char buf[32];

	auto getString = [&buf](int i)
	{
		std::snprintf(buf, sizeof(buf), "String %d", i);
		return buf;
	};

	constexpr int count = 16;
	for (int i = 0; i < count; ++i)
	{
		kokko::String s(allocator, getString(i));
		auto* pair = map.Insert(s);
		pair->second = s;
	}

	for (int i = 0; i < count; ++i)
	{
		kokko::String s(allocator, getString(i));
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
