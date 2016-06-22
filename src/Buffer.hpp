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
		this->operator=(other);
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

	Buffer& operator=(const Buffer& other)
	{
		if (this != &other)
		{
			if (other.data != nullptr && other.count > 0)
			{
				this->Allocate(other.count);

				for (SizeType i = 0; i < this->count; ++i)
					data[i] = other.data[i];
			}
			else
			{
				this->data = nullptr;
				this->count = 0;
			}
		}

		return *this;
	}

	Buffer& operator=(Buffer&& other)
	{
		// Deleting a nullptr is a no-op
		delete[] this->data;

		this->data = other.data;
		this->count = other.count;

		other.data = nullptr;
		other.count = 0;

		return *this;
	}
	
	void Allocate(SizeType count)
	{
		// Deleting a nullptr is a no-op
		delete[] this->data;

		this->count = count;

		if (count > 0)
			this->data = new T[count];
		else
			this->data = nullptr;
	}
	
	void Deallocate()
	{
		// Deleting a nullptr is a no-op
		delete[] this->data;
		
		this->data = nullptr;
		this->count = 0;
	}

	bool IsValid() { return this->data != nullptr; }
	
	T* Data() { return this->data; }
	const T* Data() const { return this->data; }
	SizeType Count() const { return this->count; }
	
	T& operator[](SizeType index) { return this->data[index]; }
	const T& operator[](SizeType index) const { return this->data[index]; }
};
