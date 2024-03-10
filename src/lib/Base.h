#ifndef BASE_H
#define BASE_H

// Compiler context

#ifdef __clang__
#define COMPILER_CLANG 1
#else
#define COMPILER_CLANG 0
#error compiler not supported
#endif

#ifdef _WIN32
#define OS_WINDOWS 1
#else
#define OS_WINDOWS 0
#ifdef __linux__
#ifdef __ANDROID__
#define OS_LINUX 0
#define OS_ANDROID 1
#else
#define OS_ANDROID 0
#define OS_LINUX 1
#endif
#else
#define OS_LINUX 0
#ifdef __APPLE__
#define OS_MAC 1
#else
#define OS_MAC 0
#error os not supported
#endif
#endif
#endif

#ifdef __amd64__
#define ARCH_X64 1
#define ARCH_X86 0
#else
#define ARCH_X64 0
#ifdef __i386__
#define ARCH_X86 1
#define ARCH_X64 0
#else
#define ARCH_X86 0
#ifdef __aarch64__
#define ARCH_ARM64 1
#else
#define ARCH_ARM64 0
#error arch not supported
#endif
#endif
#endif

// Debug

#if OS_WINDOWS == 1
#include <Windows.h>
#define SLEEP(ms) Sleep(ms);
#else
#include <unistd.h>
#define SLEEP(ms) sleep(ms);
#endif

#define WAIT_LOOP          \
  LOG_DEBUGF("waiting...") \
  while (1) {              \
    SLEEP(1);              \
  }

#include "Log.h"
void logit(const char* line, ...);
#define DEBUG_TRACE logit("*** TRACE %s:%u\n", __FILE__, __LINE__);

#include <stdlib.h>
#define ASSERT(cond)                                                          \
  if (!(cond)) {                                                              \
    logit(("Assertion failed: " #cond "\n  at %s:%u\n"), __FILE__, __LINE__); \
    abort();                                                                  \
  }
#define ASSERT_CONTEXT(cond, ctx, ...)                                     \
  if (!(cond)) {                                                           \
    logit(                                                                 \
        ("Assertion failed: " #cond "\n  at %s:%u\n  Context: " ctx "\n"), \
        __FILE__,                                                          \
        __LINE__,                                                          \
        __VA_ARGS__);                                                      \
    abort();                                                               \
  }
#define ASSERT_EQUAL(a, b, ctx, ...)                                                            \
  if (a != b) {                                                                                 \
    logit(                                                                                      \
        ("Assertion failed: " #a " (0x%x) == " #b " (0x%x)\n  at %s:%u\n  Context: " ctx "\n"), \
        (unsigned int)a,                                                                        \
        (unsigned int)b,                                                                        \
        __FILE__,                                                                               \
        __LINE__,                                                                               \
        __VA_ARGS__);                                                                           \
    abort();                                                                                    \
  }

#define LOG_INFOF(s, ...) logit(s "\n", __VA_ARGS__);
#define LOG_DEBUGF(s, ...) logit(s "\n", __VA_ARGS__);

// Scalars

#define ARRAY_COUNT(a) (sizeof(a) / sizeof(*(a)))

#include <stdbool.h>
#include <stdint.h>
typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef s8 b8;
typedef s16 b16;
typedef s32 b32;
typedef s64 b64;
typedef float f32;
typedef double f64;

// Prefixes
// g for global
// m for members
// p for pointer
// s for static (as in private)

// Suffixes
// t for typedef

// Math

#define MATH_MIN(a, b) (((a) < (b)) ? (a) : (b))
#define MATH_MAX(a, b) (((a) > (b)) ? (a) : (b))
#define MATH_CLAMP(min, n, max) (((n) < (min)) ? (min) : ((max) < (n)) ? (max) : (n))

#endif