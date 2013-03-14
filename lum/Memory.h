#pragma once

#include "Atomic.h"
#include "Platform.h"
#include <new>


#define LUM_MAKE(allocator, Type, ...) \
    ((*allocator).make<Type>([](void* mem) {new(mem) Type(__VA_ARGS__);}))

#define LUM_MAKE_ARRAY(allocator, Type, length) \
    ((*allocator).make_array<Type>(length))
    //
    // E.g.
    // >    Owned<Bar>  p_bar = LUM_MAKE(lum::Allocator::global(), Bar);           => construct p_bar with Bar()         (exclusive ownership)
    // >    Shared<Foo> p_foo = LUM_MAKE(lum::Allocator::global(), Foo, a, b, c);  => construct p_foo with Foo(a, b, c)     (shared ownership)
    // >    Shared<Baz> p_baz = LUM_MAKE_ARRAY(allocator, Baz, 42);                => construct p_baz with 42 elements      (shared ownership)

namespace lum {


// Ptr<T>   ~ Base class for smart pointers

template<class T>
class Ptr
{
public:
    typedef T Type;

    bool is_null() const;
    unsigned capacity() const;

    operator T*() const;

protected:

    Ptr();

    void destroy();

    template<class U> void swap(Ptr<U>& other);

    T* p_;

    template<class U> friend class Ptr;
};


// Owned<T>     ~ Smart pointer with exclusive ownership semantics

template<class T>
class Owned : public Ptr<T>
{
public:

    Owned();
    Owned(Owned& other);
    ~Owned();

    void operator=(Owned other);

    template<class U> Owned<U> as();

private:

    explicit Owned(T* p);

    friend class Allocator;
};


// Shared<T>    ~ Smart pointer with shared ownership semantics

template<class T>
class Shared : public Ptr<T>
{
public:

    Shared();
    Shared(const Shared& other);
    Shared(Owned<T>& other);
    ~Shared();

    void operator=(Shared other);

    template<class U> Shared<U> as() const;

private:

    void acquire();
    unsigned release();
};


// Allocator    ~ Base class for memory allocators

class Allocator
{
public:

    virtual void* alloc(size_t bytes) = 0;
    virtual void free(void*, size_t bytes) = 0;

    template<class T, typename FUNC> Owned<T> make(FUNC init);
    template<class T> Owned<T> make_array(unsigned count);

    template<class T> static Shared<Allocator> of(const Ptr<T>& obj);
    static const Shared<Allocator>& global();

    LUM_DECL_ALIGNED(8)
    struct Block
    {
        Block(Allocator* a, size_t size, ptrdiff_t offset);
        void free();

        Shared<Allocator> allocator_;
        uint32_t          size_;
        uint32_t          offset_;
        volatile int32_t  refs_;
    };

protected:

    virtual ~Allocator() {}

    Allocator::Block* reserve(size_t bytes, size_t alignment);

    static Shared<Allocator> global_;

    friend class Ptr<Allocator>;
};


// Pad    ~ Utility class for padding structs

template<int N, int ALIGN> struct Pad;
template<int N> struct Pad<N, 16> { LUM_DECL_ALIGNED(16) char _[N]; };
template<int N> struct Pad<N,  8> { LUM_DECL_ALIGNED(8)  char _[N]; };
template<int N> struct Pad<N,  4> { LUM_DECL_ALIGNED(4)  char _[N]; };
template<int N> struct Pad<N,  2> { LUM_DECL_ALIGNED(2)  char _[N]; };
template<int N> struct Pad<N,  1> { LUM_DECL_ALIGNED(1)  char _[N]; };


// Placeholder    ~ Utility class for holding space for a type

template<class T>
struct Placeholder
{
    Pad<sizeof(T), LUM_ALIGNOF(T)> _;
};


void* align_forward(void const* p, size_t alignment);
void* align_backward(const void* p, size_t alignment);


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

inline void* align_forward(void const* p, size_t alignment )
{
    size_t address = (size_t)p;
    address += alignment-1;
    address -= address % alignment;
    return (void*)address;
}

inline void* align_backward(const void* p, size_t alignment)
{
    size_t address = (size_t)p;
    address -= address % alignment;
    return (void*)address;
}

template<class T, typename FUNC> Owned<T> Allocator::make(FUNC init)
{
    Block* block = reserve(sizeof(T), LUM_ALIGNOF(T));
    void* p = block + 1;
    init(p);
    return Owned<T>(static_cast<T*>(p));
}

template<class T> Owned<T> Allocator::make_array(unsigned count)
{
    Block* block = reserve(count*sizeof(T), LUM_ALIGNOF(T));
    void* p = block + 1;
    for (unsigned i = 0; i < count; ++i)
    {
        new((T*)p+i) T;
    }
    return Owned<T>((T*)p);
}

inline const Shared<Allocator>& Allocator::global()
{
    return global_;
}

template<class T> Shared<Allocator> Allocator::of(const Ptr<T>& obj)
{
    Shared<Allocator> allocator;
    if (!obj.is_null())
    {
        T* p = obj;
        Block* block = (Block*)p - 1;
        allocator = block->allocator_;
    }
    return allocator;
}

template<class T> Ptr<T>::Ptr() : p_(nullptr)
{
}

template<class T> void Ptr<T>::destroy()
{
    if (p_ != nullptr)
    {
        unsigned n = capacity();
        for (unsigned i = 0; i < n; ++i)
        {
            p_[i].~T();
        }

        Allocator::Block* block = (Allocator::Block*)p_ - 1;
        block->free();
    }
}

template<class T> template<class U> void Ptr<T>::swap(Ptr<U>& other)
{
    T* temp = p_;
    p_ = static_cast<T*>(other.p_);
    other.p_ = static_cast<U*>(temp);
}

template<class T> bool Ptr<T>::is_null() const
{
    return p_ == nullptr;
}

template<class T> unsigned Ptr<T>::capacity() const
{
    unsigned length = 0;
    if (p_ != nullptr)
    {
        Allocator::Block* block = (Allocator::Block*)p_ - 1;
        length = block->size_ / sizeof(T);
    }
    return length;
}

template<class T> Ptr<T>::operator T*() const
{
    return p_;
}

template<class T> Owned<T>::Owned()
{
}

template<class T> Owned<T>::Owned(Owned<T>& other)
{
    swap(other);
}

template<class T> Owned<T>::Owned(T* p)
{
    p_ = p;
}

template<class T> Owned<T>::~Owned()
{
    destroy();
}

template<class T> void Owned<T>::operator=(Owned<T> other)
{
    swap(other);
}

template<class T> template<class U> Owned<U> Owned<T>::as()
{
    Owned<U> p;
    swap(p);
    return p;
}

template<class T> Shared<T>::Shared()
{
}

template<class T> Shared<T>::Shared(const Shared<T>& other)
{
    p_ = other.p_;
    acquire();
}

template<class T> Shared<T>::Shared(Owned<T>& other)
{
    swap(other);
    acquire();
}

template<class T> Shared<T>::~Shared()
{
    if (release() == 0)
    {
        destroy();
    }
}

template<class T> void Shared<T>::operator=(Shared<T> other)
{
    swap(other);
}

template<class T> template<class U> Shared<U> Shared<T>::as() const
{
    Shared<U> r;
    Shared<T> dupe(*this);
    r.swap(dupe);
    return r;
}

template<class T> void Shared<T>::acquire()
{
    if (p_ != nullptr)
    {
        Allocator::Block* block = (Allocator::Block*)p_ - 1;
        atomic_increment(block->refs_);
    }
}

template<class T> unsigned Shared<T>::release()
{
    if (p_ != nullptr)
    {
        Allocator::Block* block = (Allocator::Block*)p_ - 1;
        return atomic_decrement(block->refs_);
    }
    return 0;
}

} // lum
