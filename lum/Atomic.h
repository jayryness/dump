#pragma once

#include "Platform.h"
#include <stdint.h>

#if defined(_MSC_VER)

namespace lum {

inline int32_t atomic_increment(int32_t volatile& n)
{
    return InterlockedIncrement((long*)&n);
}

inline int64_t atomic_increment(int64_t volatile& n)
{
    return InterlockedIncrement64((__int64*)&n);
}

inline int32_t atomic_decrement(int32_t volatile& n)
{
    return InterlockedDecrement((long*)&n);
}

inline int64_t atomic_decrement(int64_t volatile& n)
{
    return InterlockedDecrement64((__int64*)&n);
}

inline int32_t atomic_exchange(int32_t volatile& dest, int32_t source)
{
    return InterlockedExchange((long*)&dest, source);
}

inline int64_t atomic_exchange(int64_t volatile& dest, int64_t source)
{
    return InterlockedExchange64((__int64*)&dest, source);
}

inline void* atomic_exchange(void* volatile& dest, void* source)
{
    return InterlockedExchangePointer(&dest, source);
}

}   // lum

#else

#error Unsupported platform.

#endif
