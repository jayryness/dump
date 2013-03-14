
#if defined(_MSC_VER)
#   define NOMINMAX
#   include <Windows.h>
#   undef NOMINMAX
#   define LUM_RESTRICT __restrict
#   define LUM_ALIGNOF(T) __alignof(T)
#   define LUM_DECL_ALIGNED(x) __declspec(align(x))
#   define LUM_FORCE_INLINE __forceinline
#   define LUM_DEBUG_BREAK() __debugbreak()
#else
#   error Unsupported platform.
#endif

#define LUM_NOP ((void)0)

#if defined(_WIN64)
#   define LUM_MIN_ALIGNMENT 16
#elif defined(_WIN32)
#   define LUM_MIN_ALIGNMENT 8
#else
#   error Unsupported platform.
#endif

#ifdef NDEBUG
#   define LUM_ASSERT(condition) ((void)0)
#   define LUM_VERIFY(condition, msg) ((condition) ? LUM_NOP : lum::Fail(__FILE__, __LINE__, msg))
#else
#   define LUM_ASSERT(condition) ((condition) ? LUM_NOP : LUM_DEBUG_BREAK())
#   define LUM_VERIFY(condition, msg) LUM_ASSERT(condition)
#endif

#ifndef LUM_FAIL_DECLARED
#define LUM_FAIL_DECLARED
namespace lum { void Fail(const char*, int, const char*); }
#endif
