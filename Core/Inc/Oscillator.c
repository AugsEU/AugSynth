#include "Oscillator.h"
#include "Constants.h"


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


/// @brief Get value of oscillator as a squarewave
float_t OscSquare(Oscillator_t* osc, float_t dt)
{
    float_t phase = osc->mPhase;

    float_t value = phase < 0.5f ? 1.0f : -1.0f;
    if(phase < dt || phase > 1.0f - dt)
    {
        value += PolyBlep(phase, dt);
    }
    else
    {
        phase += 0.5f;
        if (phase > 1.0f)
        {
            phase -= 1.0f;
        }
        value -= PolyBlep(phase, dt);
    }

    return value;
}

// float_t OscSquare(Oscillator_t* osc, float_t dt)
// {
//     float_t phase = osc->mPhase;
//     float_t polyBlep = 0.0f;

//     if (phase <= 0.5f)
//     {
//         if (phase < dt)
//         {
//             polyBlep = phase / dt;
//             polyBlep = 2.0f * polyBlep - polyBlep * polyBlep - 1.0f;
//         }
//         else if (phase >= 0.5f - dt)
//         {
//             polyBlep = (0.5f - phase) / dt;
//             polyBlep = 2.0f * polyBlep - polyBlep * polyBlep - 1.0f;
//         }

//         return 1.0f + polyBlep;
//     }

//     if (phase <= 0.5f + dt)
//     {
//         polyBlep = (phase-0.5f) / dt;
//         polyBlep =  polyBlep * polyBlep + 2.0f * polyBlep - 1.0f;
//     }
//     else if (phase > 1.0f - dt)
//     {
//         polyBlep = (1.0f - phase) / dt;
//         polyBlep =  polyBlep * polyBlep + 2.0f * polyBlep - 1.0f;
//     }

//     return -1.0f + polyBlep;
// }


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
