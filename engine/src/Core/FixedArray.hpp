#pragma once

#include <cassert>
#include <cstddef>
#include <new>
#include <utility>

#include "Core/ArrayView.hpp"

namespace kokko
{

template <typename ValueType, size_t Capacity>
class FixedArray
{
private:
    size_t count;
    ValueType data[Capacity];

public:
    constexpr unsigned int GetCapacity() const { return Capacity; }
    FixedArray() : count(0)
    {
    }

    ~FixedArray()
    {
        for (size_t i = 0; i < count; ++i)
            data[i].~ValueType();
    }

    size_t GetCount() const { return this->count; }

    ValueType* GetData() { return data; }
    const ValueType* GetData() const { return data; }

    ValueType& GetFront() { return this->data[0]; }
    const ValueType& GetFront() const { return this->data[0]; }

    ValueType& GetBack() { return this->data[this->count - 1]; }
    const ValueType& GetBack() const { return this->data[this->count - 1]; }

    ArrayView<ValueType> GetView() { return ArrayView(data, count); }
    ArrayView<const ValueType> GetView() const { return ArrayView(data, count); }

    ValueType& At(unsigned int index) { return data[index]; }
    const ValueType& At(unsigned int index) const { return data[index]; }

    ValueType& operator[](unsigned int index) { return data[index]; }
    const ValueType& operator[](unsigned int index) const { return data[index]; }

    ValueType& PushBack()
    {
        assert(count < Capacity);
        ValueType* ptr = new (data + count) ValueType();
        ++count;
        return *ptr;
    }

    void PushBack(const ValueType& value)
    {
        assert(count < Capacity);
        new (data + count) ValueType(value);
        ++count;
    }

    void PopBack()
    {
        --(count);

        data[count].~ValueType();
    }

    void Clear()
    {
        for (size_t i = 0; i < count; ++i)
            data[i].~ValueType();

        count = 0;
    }

    class Iterator
    {
    private:
        ValueType* iterator;

        explicit Iterator(ValueType* itr) : iterator(itr)
        {
        }

        friend class FixedArray;

    public:
        Iterator& operator++()
        {
            iterator++;
            return *this;
        }

        Iterator operator++(int)
        {
            Iterator temp(*this);
            ++(*this);
            return temp;
        }

        ValueType& operator*() { return *iterator; }
        const ValueType& operator*() const { return *iterator; }
        ValueType& operator->() { return operator*(); }
        const ValueType& operator->() const { return operator*(); }

        bool operator!=(const Iterator& other) { return iterator != other.iterator; }
    };

    Iterator begin() { return Iterator(data); }
    Iterator end() { return Iterator(data + count); }
};

} // namespace kokko
