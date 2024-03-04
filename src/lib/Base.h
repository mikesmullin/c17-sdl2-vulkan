#ifndef BASE_H
#define BASE_H

// Debug

#include <stdio.h>
#define DEBUG_TRACE printf("*** TRACE %s:%u\n", __FILE__, __LINE__);

#define ASSERT(c) \
  if (!(c)) {     \
    abort();      \
  }
#define ASSERT_ERROR(cond, code, errs, ...)   \
  if (!(cond)) {                              \
    fprintf(stderr, errs[code], __VA_ARGS__); \
    abort();                                  \
  }

#define RETURN_ERROR(code, errs, ...)       \
  fprintf(stderr, errs[code], __VA_ARGS__); \
  return code;

#define LOG_INFOF(s, ...) printf(s, __VA_ARGS__);
#define LOG_DEBUGF(s, ...) printf(s, __VA_ARGS__);

// Scalars

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
#define MATH_CLAMP(min, n, max) (((n) < (min)) ? (min) : ((max) < (n)) ? (max) : (x))

#endif