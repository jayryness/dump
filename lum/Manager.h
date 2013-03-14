#pragma once

#include "Buffer.h"
#include "Atomic.h"

namespace lum {


typedef uint64_t Uid;

template<class T>
class Manager
{
public:

    //Lock<T> acquire(Uid uid);

protected:

    enum { SlotMask = 0x00ffffff };

    struct Item
    {
        Placeholder<T> data_;
        Uid uid_;
    };
    struct Index
    {
        uint32_t slot_;
        uint32_t counter_;
    };

    Manager(const Shared<Allocator>& allocator);
    Item* add_item();
    void  remove_item(Item*);
    Item* fetch(Uid);

    Uid uid_of(Item*);

    Buffer<Item>    items_;
    Buffer<Index>   indices_;

    uint32_t manager_id_;
    unsigned first_free_;
};

uint32_t next_global_serial();


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template<class T> Manager<T>::Manager(const Shared<Allocator>& allocator)
    : items_(allocator, 0)
    , indices_(allocator, 0)
    , manager_id_(next_global_serial())
    , first_free_(-1)
{
}

template<class T> typename Manager<T>::Item* Manager<T>::add_item()
{
    unsigned key = first_free_;
    if (key >= indices_.length())
    {
        key = indices_.length();
        Index* index = &indices_[key];
        index->slot_ = -1;
        index->counter_ = 0;
    }
    Index* indices = indices_.start();
    Index& index = indices[key];

    first_free_ = index.slot_;
    index.slot_ = items_.length();

    Item* item = &items_[index.slot_];
    item->uid_ = key + manager_id_ * (SlotMask+1) + index.counter_ * 0x100000000LL;
    return item;
}

template<class T> void Manager<T>::remove_item(Item* item)
{
    unsigned key = item->uid_ & SlotMask;
    if (key < indices_.length())
    {
        Index* index = indices_.start() + key;
        if ((item->uid_ >> 32) == index->counter_)
        {
            Item* items = items_.start();
            *item = items[items_.length()-1];
            items_.remove(1);

            indices_[item->uid_ & SlotMask].slot_ = index->slot_;

            ++index->counter_;
            index->slot_ = first_free_;
            first_free_ = key;

            return;
        }
    }
    LUM_VERIFY(false, "Attempted removal of stale item from manager.");
}

template<class T> typename Manager<T>::Item* Manager<T>::fetch(Uid uid)
{
    unsigned key = uid & SlotMask;
    if (key < indices_.length())
    {
        Index* index = indices_.start() + key;
        Item* item = items_.start() + index->slot_;
        if (item->uid_ == uid)
        {
            return item;
        }
    }
    return nullptr;
}

}   // lum
