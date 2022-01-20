DEBUG_OPTIMIZE_ON

// math_1 = 99% of f32 math library (vectors, matrices, etc.)
// fixed_math = fix64 math library (fixed point)
// math_2 = 1% of f32 math library that depends upon fix64 (mostly conversion utilities)

#include "math_1.h"
#include "fixed_math.h"
#include "math_2.h"

DEBUG_OPTIMIZE_OFF
