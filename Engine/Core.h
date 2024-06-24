#pragma once

#ifdef ENGINE_BUILD_DLL
#define ENGINE_API __declspec(dllexport)
#else
#define ENGINE_API __declspec(dllimport)
#endif

#ifdef ENGINE_ENABLE_ASSERTS
#define ENGINE_ASSERT(x, ...)                         \
  {                                                   \
    if (!(x))                                         \
  }                                                   \
  {                                                   \
    BITTY_CORE("Assertion Failed: {0}", __VA_ARGS__); \
    __debugbreak();                                   \
  }                                                   \
  }
#define ENGINE_CORE_ASSERT(x, ...)                          \
  {                                                         \
    if (!(x))                                               \
  }                                                         \
  {                                                         \
    BITTY_CORE_ERROR("Assertion Failed: {0}", __VA_ARGS__); \
    __debugbreak();                                         \
  }                                                         \
  }
#else
#define ENGINE_ASSERT(x, ...)
#define ENGINE_CORE_ASSERT(x, ...)
#endif

#define BIT(x) (1 << (x))