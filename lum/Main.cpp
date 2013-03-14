#include <cstdio>
#include "Buffer.h"
#include "Manager.h"

namespace lum {
    void Fail(const char*, int, const char* msg)
    {
        printf("\n\n-------- ERROR ----------\n\n");
        printf(msg);
        printf("\n\n-- press Enter to exit --\n");
        getchar();
        exit(0);
    }
}

struct Bye
{
    Bye()
        : a_(77)
        , b_(88)
    {
    }
    Bye(int a, int b)
        : a_(a)
        , b_(b)
    {
    }
    ~Bye()
    {
        printf("Bye\n");
    }
    int a_;
    int b_;
};

class ByeManager : public lum::Manager<Bye>
{
public:
    ByeManager()
        : Manager<Bye>(lum::Allocator::global())
    {
    }

    lum::Uid create()
    {
        Item* item = add_item();
        new (&item->data_) Bye();
        return item->uid_;
    }
    void destroy(lum::Uid uid)
    {
        Item* item = fetch(uid);
        if (item != nullptr)
        {
            Bye* bye = (Bye*)&item->data_;
            bye->~Bye();
            remove_item(item);
        }
    }

};

void main()
{
    printf("Hello\n");
    auto allocator = lum::Allocator::global();
    auto bye = LUM_MAKE(allocator, Bye, 1, 2);

    lum::Shared<Bye> byebye = bye;

    lum::Shared<Bye> many_byes = LUM_MAKE_ARRAY(allocator, Bye, 13);

    lum::Buffer<int> int_buf(allocator, 5);
    //int_buf[42];
    for (int i = 0; i < 42; i++)
    {
        int_buf[i] = i;
    }

    ByeManager bye_manager;
    {
        lum::Uid a = bye_manager.create();
        lum::Uid b = bye_manager.create();
        lum::Uid c = bye_manager.create();
        bye_manager.destroy(b);
        b = bye_manager.create();
        bye_manager.destroy(b);
        bye_manager.destroy(a);
        bye_manager.destroy(c);
    }

    //LUM_VERIFY(false, "test");
}
