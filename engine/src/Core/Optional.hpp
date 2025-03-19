#pragma once

#include <utility>

namespace kokko
{

template <typename T>
class Optional
{
private:
	T value;
	bool hasValue;

public:
	Optional() :
		hasValue(false)
	{
	}

	Optional(const T& v) :
		value(v),
		hasValue(true)
	{
	}

	Optional(T&& v) :
		value(std::move(v)),
		hasValue(true)
	{
	}

	~Optional() = default;

	Optional& operator=(const T& v)
	{
		value = v;
		hasValue = true;
		return *this;
	}

	Optional& operator=(T&& v)
	{
		value = std::move(v);
		hasValue = true;
		return *this;
	}

	operator bool() const
	{
		return HasValue();
	}

	bool HasValue() const
	{
		return hasValue;
	}

	T& GetValue()
	{
		return value;
	}

	const T& GetValue() const
	{
		return value;
	}
};

} // namespace kokko
