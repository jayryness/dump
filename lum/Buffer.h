#pragma once

#include "Memory.h"

namespace lum {


template<class T>
class Buffer
{
public:
    Buffer(const Shared<Allocator>& allocator, unsigned length=0);
    ~Buffer();

    T* start() const;
    T* end() const;

    template<class FUNC> void for_each(int start, int end, FUNC func);

    unsigned length() const;

    T& operator[](int i);
    void remove(unsigned count);

private:

    void realloc(unsigned capacity);

    Owned<T> data_;
    unsigned length_;
};


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template<class T> Buffer<T>::Buffer(const Shared<Allocator>& allocator, unsigned length)
    : length_(length)
{
    data_ = LUM_MAKE_ARRAY(allocator, T, length_);
}

template<class T> Buffer<T>::~Buffer()
{
}

template<class T> T* Buffer<T>::start() const
{
    return data_;
}

template<class T> T* Buffer<T>::end() const
{
    return (T*)data_ + length_;
}

template<class T> template<class FUNC> void Buffer<T>::for_each(int start, int end, FUNC func)
{
    LUM_ASSERT(start >= 0 && end <= length_);
    T* data = data_;
    for (int i = start; i < end; i++)
    {
        func(data[i]);
    }
}

template<class T> unsigned Buffer<T>::length() const
{
    return length_;
}

template<class T> LUM_FORCE_INLINE T& Buffer<T>::operator[](int i)
{
    LUM_ASSERT(i >= 0);
    if (unsigned(i) >= length_)
    {
        if (unsigned(i) >= data_.capacity())
        {
            realloc(i*2 + 1);
        }
        length_ = i + 1;
    }

    T* p = data_;
    return p[i];
}

template<class T> void Buffer<T>::remove(unsigned count)
{
    LUM_ASSERT(length_ >= count);
    length_ -= count;
}

template<class T> void Buffer<T>::realloc(unsigned capacity)
{
    LUM_ASSERT(capacity >= length_);
    Allocator* allocator = Allocator::of(data_);
    Owned<T> data(data_);
    data_ = allocator->make_array<T>(capacity);
    T* dest = data_;
    T* src = data;
    for (unsigned i = 0; i < length_; ++i)
    {
        dest[i] = src[i];
    }
}

}   // lum
