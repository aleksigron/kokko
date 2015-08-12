#pragma once

template <typename T>
class Buffer
{
public:
	using SizeType = unsigned long;

private:
	T* data;
	SizeType count;
	
public:
	Buffer(): data(nullptr), count(0)
	{
	}

	Buffer(const Buffer& other): data(nullptr), count(0)
	{
		if (this != &other && other.data != nullptr && other.count > 0)
		{
			this->Allocate(other.count);

			for (SizeType i = 0; i < this->count; ++i)
				data[i] = other.data[i];
		}
	}

	Buffer(Buffer&& other): data(other.data), count(other.count)
	{
		other.data = nullptr;
		other.count = 0;
	}

	~Buffer()
	{
		// Deleting a nullptr is a no-op
		delete[] this->data;
	}
	
	void Allocate(SizeType count)
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

	inline bool IsValid() { return this->data != nullptr; }
	
	inline T* Data() { return this->data; }
	inline const T* Data() const { return this->data; }
	inline SizeType Count() const { return this->count; }
	
	inline T& operator[](SizeType index) { return this->data[index]; }
	inline const T& operator[](SizeType index) const { return this->data[index]; }
};