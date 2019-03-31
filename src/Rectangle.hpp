#pragma once

#include "Vec2.hpp"

template<typename T>
struct Rectangle
{
	Vec2<T> position;
	Vec2<T> size;

	Rectangle()
	{
	}

	Rectangle(T pos_x, T pos_y, T width, T height) :
	position(pos_x, pos_y),
	size(width, height)
	{
	}

	Rectangle(const Vec2<T>& position, const Vec2<T>& size) : position(position), size(size)
	{
	}

	bool Contains(Vec2<T> point)
	{
		Vec2f end = position + size;

		bool horizontal = point.x >= position.x && point.x < end.x;
		bool vertical = point.y >= position.y && point.y < end.x;

		return horizontal && vertical;
	}

	bool Intersects(const Rectangle& other)
	{
		Vec2<T> this_end = position + size;
		Vec2<T> other_end = other.position + other.size;

		bool leftIsWithin = position.x >= other.position.x && position.x < other_end.x;
		bool rightIsWithin = this_end.x >= other.position.x && this_end.x < other_end.x;
		bool leftRightOut = position.x < other.position.x && this_end.x >= other_end.x;

		bool topIsWithin = position.y >= other.position.y && position.y < other_end.y;
		bool bottomIsWithin = this_end.y >= other.position.y && this_end.y < other_end.y;
		bool topBottomOut = position.y < other.position.y && this_end.y >= other_end.y;

		bool horizontal = leftIsWithin || rightIsWithin || leftRightOut;
		bool vertical = topIsWithin || bottomIsWithin || topBottomOut;

		return horizontal && vertical;
	}
};

using Rectanglef = Rectangle<float>;
using Rectanglei = Rectangle<int>;
