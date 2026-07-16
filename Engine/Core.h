#pragma once

#include <intrin.h>
#include "src/Log.h"

#ifdef ENGINE_BUILD_DLL
#define ENGINE_API __declspec(dllexport)
#else
#define ENGINE_API __declspec(dllimport)
#endif

#ifdef ENGINE_ENABLE_ASSERTS
#define ENGINE_ASSERT(x, msg)                    \
  do                                              \
  {                                               \
    if (!(x))                                     \
    {                                             \
      ENGINE_LOG_ERROR("Assertion Failed: %s", msg); \
      __debugbreak();                             \
    }                                             \
  } while (0)
#define ENGINE_CORE_ASSERT(x, msg) ENGINE_ASSERT(x, msg)
#else
#define ENGINE_ASSERT(x, msg)
#define ENGINE_CORE_ASSERT(x, msg)
#endif

#define BIT(x) (1 << (x))