#pragma once

#include "Vec2.hpp"

struct Rectangle
{
	Vec2f position;
	Vec2f size;

	Rectangle()
	{
	}

	Rectangle(float pos_x, float pos_y, float width, float height) :
	position(pos_x, pos_y),
	size(width, height)
	{
	}

	Rectangle(const Vec2f& position, const Vec2f& size) : position(position), size(size)
	{
	}

	bool Contains(Vec2f point)
	{
		Vec2f end = position + size;

		bool horizontal = point.x >= position.x && point.x < end.x;
		bool vertical = point.y >= position.y && point.y < end.x;

		return horizontal && vertical;
	}

	bool Intersects(const Rectangle& other)
	{
		Vec2f this_end = position + size;
		Vec2f other_end = other.position + other.size;

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
