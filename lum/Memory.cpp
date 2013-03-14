#include "Memory.h"
#include <algorithm>

namespace lum {


class Mallocator : public Allocator
{
public:
    virtual void* alloc(size_t bytes)
    {
        return malloc(bytes);
    }
    virtual void free(void* p, size_t bytes)
    {
        ::free(p);
    }
};

struct Bootstrap
{
    Owned<Allocator> null_;
    Allocator::Block header_;   // Must immediately precede mallocator in memory.
    Mallocator mallocator_;

    Bootstrap() : header_(null_, 0, 0)
    {
        ++header_.refs_;    // An extra ref to prevent free()ing the mallocator.
    }
};
static Bootstrap bootstrap__;
Shared<Allocator> Allocator::global_( Owned<Allocator>(&bootstrap__.mallocator_) );

Allocator::Block::Block(Allocator* a, size_t size, ptrdiff_t offset)
    : allocator_((Shared<Allocator>&)a)
    , size_(uint32_t(size))
    , offset_(uint32_t(offset))
    , refs_(0)
{
    LUM_ASSERT(size <= 0xffffffff);
}

void Allocator::Block::free()
{
    LUM_VERIFY(refs_ == 0, "Memory block destroyed while still referenced.");

    Shared<Allocator> allocator = allocator_;

    void* base = (char*)this + sizeof(Block) - offset_;
    size_t size = size_ + offset_;

    this->~Block();

    Allocator* a = allocator;
    a->free(base, size);
}

Allocator::Block* Allocator::reserve(size_t bytes, size_t alignment)
{
    const size_t cushion = std::max(alignment, (size_t)LUM_MIN_ALIGNMENT) - LUM_MIN_ALIGNMENT;
    void* base = alloc(cushion + sizeof(Block) + bytes);

    LUM_VERIFY(base != nullptr, "Memory allocation failed.");

    void* payload = align_forward((char*)base + sizeof(Block), alignment);

    Block* block = (Block*)payload - 1;
    new (block) Block(this, bytes, (char*)payload - (char*)base);

    return block;
}

}   // lum
