#pragma once

#include <cstddef>

template <typename T>
class Buffer
{
private:
	T* data = nullptr;
	std::size_t count = 0;
	
public:
	~Buffer()
	{
		// Deleting a nullptr is a no-op
		delete[] this->data;
	}
	
	void Allocate(std::size_t count)
	{
		// Deleting a nullptr is a no-op
		delete[] this->data;
		
		this->data = new T[count];
		this->count = count;
	}
	
	void Deallocate()
	{
		// Deleting a nullptr is a no-op
		delete[] this->data;
		
		this->data = nullptr;
		this->count = 0;
	}
	
	T* Data() { return this->data; }
	const T* Data() const { return this->data; }
	std::size_t Count() const { return this->count; }
	
	T& operator[](std::size_t index) { return this->data[index]; }
	const T& operator[](std::size_t index) const { return this->data[index]; }
};