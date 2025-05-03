#ifndef AUG_MATH_H
#define AUG_MATH_H

#include <math.h>

float_t FastUnitExp(float_t x)
{
    const float_t A = 0.532061f;
    const float_t B = 1.10892f;
    const float_t C = 1.0f;

    float_t x2 = A * x;
    x2 += B;
    x *= x2;
    return x + C;
}

#endif // AUG_MATH_H