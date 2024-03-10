#ifndef TIMER_H
#define TIMER_H

// A cycle timer is a kind of monotonic clock
// this is a cross-platform implementation
// it provides a single function `u64 Now()` which returns time since PC power on
// it uses the RDTSC (Read Time-Stamp Counter) instruction to count CPU cycles
// it is fast because it avoids a system call
// it has some limitations
// - performance will vary by cpu
// - consistency is not guaranteed across threads
// - you can measure milliseconds, but not exactly cycles-per-opcode

#include "Base.h"

extern u64 CYCLES_PER_SECOND;
extern u64 CYCLES_PER_MILLISECOND;

void Timer__MeasureCycles();
f32 Timer__NowSeconds();
u32 Timer__NowMilliseconds();

#if OS_MAC == 1
#include <mach/mach_time.h>
#endif

// #if OS_WINDOWS == 1
// #include <time.h>
// #endif

#if OS_EMSCRIPTEN == 1
#include <emscripten.h>
#endif

inline __forceinline u64 Now() {
#if OS_MAC == 1
  // NOTICE: this counter pauses while Mac sleeps
  return (u64(mach_absolute_time();

#elif OS_EMSCRIPTEN == 1
  return (u64)(emscripten_get_now() * 1e+6);

#elif defined(__i386__)
  u64 ret;
  __asm__ volatile("rdtsc" : "=A"(ret));
  return ret;

#elif defined(__x86_64__) || defined(__amd64__)
  u64 low, high;
  __asm__ volatile("rdtsc" : "=a"(low), "=d"(high));
  return (u64)((high << 32) | low);

#else
#error OS not supported by cycle timer implementation

#endif
}

#endif  // TIMER_H