#ifndef AUG_MATH_H
#define AUG_MATH_H

#include <math.h>
#include <stdint.h>

uint32_t gSeed;

uint32_t GetNextRand()
{
    gSeed *= 1664525;
    gSeed += 1013904223;
    return gSeed;
}

float_t FastUnitExp(float_t x)
{
    float_t g = 0.75 * x;
    g += 0.25f;
    x *= g;
    return x + 1.0f;
}

float_t ComputeLfoMult(float_t lfoValue, float_t lfoAmount)
{
    if(signbit(lfoAmount)) // If negative
    {
        lfoValue += 1.0f;
    }
    else
    {
        lfoValue -= 1.0f;
    }

    return 1.0f + lfoAmount * lfoValue;
}

float_t ComputeLoudnessMult(float_t loudness, float_t amount)
{
    loudness -= 1.0f;
    return 1.0f + loudness * amount;
}

float_t ShapeWave(float_t waveValue, float_t waveShape)
{
    float_t g = waveValue * waveValue;
    g *= g;
    // g *= g;

    g *= waveShape;
    g += (1.0f - waveShape);

    return waveValue * g;
}

#define DRIVE_K (1.0f)
#define DRIVE_M (4.0f*DRIVE_K + 1.0f)

#define DRIVE_A (DRIVE_K*DRIVE_M - DRIVE_K)
#define DRIVE_B ((DRIVE_K+1.0f)*(1.0f-DRIVE_M))
#define DRIVE_C (DRIVE_M)

float_t DrivenSample(float_t sample)
{
    float_t p;
    if(signbit(sample))
    {
        sample = -sample;
        if(sample > 1.0f)
        {
            return -1.0f;
        }
        p = DRIVE_B + DRIVE_A*sample;
        p *= p;
        p += DRIVE_C;
        return -p*sample;
    }

    if(sample > 1.0f)
    {
        return 1.0f;
    }

    p = DRIVE_B + DRIVE_A*sample;
    p *= p;
    p += DRIVE_C;
    return p*sample;
}

#endif // AUG_MATH_H