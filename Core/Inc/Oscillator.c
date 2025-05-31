#include "Oscillator.h"
#include "Constants.h"

#define ACCURATE_SINE 0

float PolyBlep(float t, float dt)
{
    if (t < dt)
    {
        t /= dt;
        return 2.0f * t - t * t - 1.0f;
    }
    else if (t > 1.0f - dt)
    {
        t = (t - 1.0f) / dt;
        return t*t + 2.0f*t + 1.0f;
    }

    return 0.0f;
}


/// @brief Initialise an oscillator 
void OscInit(Oscillator_t* osc)
{
    osc->mPhase = 0.0f;
}



/// @brief Increment phase of oscillator by 1 sample.
void OscPhaseInc(Oscillator_t* osc, float_t phaseInc)
{
    osc->mPhase += phaseInc;

	if (osc->mPhase >= 1.0f)
    {
		osc->mPhase -= 1.0f;
    }
}




/// @brief Get value of oscillator as saw tooth.
float_t OscSawTooth(Oscillator_t* osc, float_t dt)
{
    float_t phase = osc->mPhase;

    phase += 0.5;
    if (phase >= 1.0f) 
    {
        phase -= 1.0f;
    }

    float_t saw = 2.0f * phase - 1.0f;
    return saw - PolyBlep(phase, dt);
}


/// @brief Get value of oscillator as a organ type wave
float_t OscOrgan(Oscillator_t* osc, float_t shape)
{
    const float_t DT = 0.1f;
    shape += 0.45f;
    shape *= 0.52f;
    float_t phase = osc->mPhase;

    float_t value = phase < shape ? 1.0f : -1.0f;
    if(phase < DT)
    {
        value = phase * (1.0f / DT);
        phase = 2.0f - value;
        value *= phase;
    }
    else if(phase < shape - DT)
    {
        value = 1.0f;
    }
    else if (phase < shape)
    {
        value = phase - shape;
        value *= (1.0f / DT);
        value += 2.0f;
        phase = 2.0f - value;
        value *= phase;
    }
    else if(phase < shape + DT)
    {
        value = phase - shape;
        value *= (1.0f / DT);
        phase = value - 2.0f;
        value *= phase;
    }
    else if (phase < 1.0f - DT)
    {
        value = -1.0f;
    }
    else
    {
        value = phase-1;
        value *= (1.0f / DT);
        value += 2.0f;
        phase = value - 2.0f;
        value *= phase;
    }

    return value;
}

/// @brief Square 
float_t OscSquareBLEP(Oscillator_t* osc, float_t dt, float_t shape)
{
    shape += 0.255f;
    shape *= 0.6125f;
    float_t phase = osc->mPhase;

    float_t value = phase < shape ? 1.0f : -1.0f;
    if(phase < dt)
    {
        value = phase / dt;
        phase = 2.0f - value;
        value *= phase;
    }
    else if(phase < shape - dt)
    {
        value = 1.0f;
    }
    else if (phase < shape)
    {
        value = phase - shape;
        value /= dt;
        value += 2.0f;
        phase = 2.0f - value;
        value *= phase;
    }
    else if(phase < shape + dt)
    {
        value = phase - shape;
        value /= dt;
        phase = value - 2.0f;
        value *= phase;
    }
    else if (phase < 1.0f - dt)
    {
        value = -1.0f;
    }
    else
    {
        value = phase-1;
        value /= dt;
        value += 2.0f;
        phase = value - 2.0f;
        value *= phase;
    }

    return value;
}

/// @brief Compute wave for low frequency oscillator
float_t OscSquareLF(Oscillator_t* osc)
{
    const float_t DT = 0.01f;
    float_t phase = osc->mPhase;

    float_t value = phase < 0.5f ? 1.0f : -1.0f;
    if(phase < DT)
    {
        value = phase * (1.0f / DT);
        phase = 2.0f - value;
        value *= phase;
    }
    else if(phase < 0.5f - DT)
    {
        value = 1.0f;
    }
    else if (phase < 0.5f)
    {
        value = phase - 0.5f;
        value *= (1.0f / DT);
        value += 2.0f;
        phase = 2.0f - value;
        value *= phase;
    }
    else if(phase < 0.5f + DT)
    {
        value = phase - 0.5f;
        value *= (1.0f / DT);
        phase = value - 2.0f;
        value *= phase;
    }
    else if (phase < 1.0f - DT)
    {
        value = -1.0f;
    }
    else
    {
        value = phase-1;
        value *= (1.0f / DT);
        value += 2.0f;
        phase = value - 2.0f;
        value *= phase;
    }

    return value;
}

float_t OscSquareFast(Oscillator_t* osc)
{
    return osc->mPhase < 0.5f ? 1.0f : -1.0f;
}

#if ACCURATE_SINE
/// @brief Get value of oscillator as sine.
float_t OscSine(Oscillator_t* osc)
{
    const float A = 73.69528f;
    const float B = -41.11008f;
    const float C = 6.28155;

    float_t x = osc->mPhase;
    float_t x2;
    float_t x4;
    
    if (x < 0.25f)
    {
        x2 = x * x;
        x4 = x2 * x2;

        x2 *= B;
        x2 += C;

        x4 *= A;
        x4 += x2;

        x *= x4;
    }
    else if (x < 0.5f)
    {
        x = 0.5f - x;
        x2 = x * x;
        x4 = x2 * x2;

        x2 *= B;
        x2 += C;

        x4 *= A;
        x4 += x2;

        x *= x4;
    }
    else if (x < 0.75f)
    {
        x -= 0.5f;

        x2 = x * x;
        x4 = x2 * x2;

        x2 *= B;
        x2 += C;

        x4 *= -A;
        x4 -= x2;

        x *= x4;
    }
    else
    {
        x = 1.0f - x;

        x2 = x * x;
        x4 = x2 * x2;

        x2 *= B;
        x2 += C;

        x4 *= -A;
        x4 -= x2;

        x *= x4;
    }

    return x;
}
#else
/// @brief Get value of oscillator as sine.
float_t OscSine(Oscillator_t* osc)
{
    float_t x = osc->mPhase;
    float_t x2;
    
    if (x < 0.5f)
    {
        x2 = x * x;
        x2 *= 2.0f;
        x -= x2;
        x *= 8.0f;
        return x;
    }
    else 
    {
        x -= 0.5f;
        x2 = x * x;
        x2 *= 2.0f;
        x -= x2;
        x *= -8.0f;
        return x;
    }
}
#endif // ACCURATE_SINE
