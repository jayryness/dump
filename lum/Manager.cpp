#include "Manager.h"
#include "Atomic.h"

namespace lum {

uint32_t next_global_serial()
{
    static volatile int32_t serial = 0;
    atomic_increment(serial);
    return (uint32_t)serial;
}

}   // lum
