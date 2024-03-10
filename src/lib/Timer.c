#include "Timer.h"

#include "Base.h"

u64 CYCLES_PER_SECOND;
u64 CYCLES_PER_MILLISECOND;

void Timer__MeasureCycles() {
  u64 start = Now();
  SLEEP(1000)
  u64 end = Now();
  CYCLES_PER_SECOND = end - start;
  CYCLES_PER_MILLISECOND = CYCLES_PER_SECOND / 1000;
}

f32 Timer__NowSeconds() {
  return (f32)Now() / CYCLES_PER_SECOND;
}

u32 Timer__NowMilliseconds() {
  return Now() / CYCLES_PER_MILLISECOND;
}