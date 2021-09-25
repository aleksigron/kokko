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
		static const int count = 24;
		int values[count];
		for (int i = 0; i < count; ++i)
			values[i] = i;

		queue.Push(values, count);

		for (int i = 0; i < count; ++i)
			CHECK(queue[i] == i);

		CHECK(queue.GetCount() == count);
	}
}

TEST_CASE("Queue.PushPop")
{
	Allocator* allocator = Allocator::GetDefault();
	Queue<int> queue(allocator);

	// 24 pushes and 1 pop

	static const int count = 24;
	int values[count];
	for (int i = 0; i < count; ++i)
		values[i] = i;

	queue.Push(values, count);

	CHECK(queue.Pop() == 0);

	CHECK(queue[0] == 1);
	CHECK(queue.GetCount() == 23);

	// 10 pushes and 10 pops

	queue.Push(24);
	CHECK(queue.Pop() == 1);
	queue.Push(25);
	CHECK(queue.Pop() == 2);
	queue.Push(26);
	CHECK(queue.Pop() == 3);
	queue.Push(27);
	CHECK(queue.Pop() == 4);
	queue.Push(28);
	CHECK(queue.Pop() == 5);
	queue.Push(29);
	CHECK(queue.Pop() == 6);
	queue.Push(30);
	CHECK(queue.Pop() == 7);
	queue.Push(31);
	CHECK(queue.Pop() == 8);
	queue.Push(32);
	CHECK(queue.Pop() == 9);
	queue.Push(33);
	CHECK(queue.Pop() == 10);

	CHECK(queue.GetCount() == 23);
}
