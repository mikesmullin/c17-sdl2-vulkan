#include "Math.h"

f64 Math__map(f64 n, f64 input_start, f64 input_end, f64 output_start, f64 output_end) {
  f64 range = 1.0 * (output_end - output_start) / (input_end - input_start);
  return output_start + range * (n - input_start);
}