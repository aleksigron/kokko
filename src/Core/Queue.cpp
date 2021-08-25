#include "Core/Queue.hpp"

#include "doctest/doctest.h"

TEST_CASE("Queue.Push")
{
	Allocator* allocator = Allocator::GetDefault();
	Queue<int> queue(allocator);

	SUBCASE("Single")
	{
		queue.Push(0);
		queue.Push(1);
		queue.Push(2);

		CHECK(queue[0] == 0);
		CHECK(queue[1] == 1);
		CHECK(queue[2] == 2);
		CHECK(queue.GetCount() == 3);
	}

	SUBCASE("SingleWithRef")
	{
		int& val0 = queue.Push();
		val0 = 0;

		int& val1 = queue.Push();
		val1 = 1;

		int& val2 = queue.Push();
		val2 = 2;

		CHECK(queue[0] == 0);
		CHECK(queue[1] == 1);
		CHECK(queue[2] == 2);
		CHECK(queue.GetCount() == 3);
	}

	SUBCASE("Multi")
	{
		int values[] = { 0, 1, 2 };
		queue.Push(values, sizeof(values) / sizeof(values[0]));

		CHECK(queue[0] == 0);
		CHECK(queue[1] == 1);
		CHECK(queue[2] == 2);
		CHECK(queue.GetCount() == 3);
	}
}
