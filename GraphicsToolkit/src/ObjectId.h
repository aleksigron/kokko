#pragma once

struct ObjectId
{
	unsigned int index;
	unsigned int innerId;
};

inline bool operator < (const ObjectId& lhs, const ObjectId& rhs)
{ return lhs.index < rhs.index; }

inline bool operator == (const ObjectId& lhs, const ObjectId& rhs)
{ return lhs.index == rhs.index && lhs.innerId == rhs.innerId; }

inline bool operator != (const ObjectId& lhs, const ObjectId& rhs)
{ return lhs.index != rhs.index || lhs.innerId != rhs.innerId; }