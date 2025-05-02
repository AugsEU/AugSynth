#include "Oscillator.h"
#include "Constants.h"


float PolyBlep(float t, float dt)
{
    if (t < dt)
    {
        t /= dt;
        return t + t - t*t - 1.0f;
    }
    else if (t > 1.0f - dt)
    {
        t = (t - 1.0f) / dt;
        return t*t + t+t + 1.0f;
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

    float saw = 2.0f * phase - 1.0f;
    return saw - PolyBlep(phase, dt);
}


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

// @TODO Test if this is faster?
// float_t OscSine(Oscillator_t* osc) 
// {
//     const float A = 73.69528f;
//     const float B = -41.11008f;
//     const float C = 6.28155f;

//     float_t x = osc->mPhase;
//     float_t adjusted_x;
//     float_t a_sign;

//     if (x < 0.5f) 
//     {
//         if (x < 0.25f)
//         {
//             adjusted_x = x;
//             a_sign = A;
//         } 
//         else
//         {
//             adjusted_x = 0.5f - x;
//             a_sign = A;
//         }
//     } 
//     else
//     {
//         if (x < 0.75f)
//         {
//             adjusted_x = x - 0.5f;
//             a_sign = -A;
//         } 
//         else
//         {
//             adjusted_x = 1.0f - x;
//             a_sign = -A;
//         }
//     }

//     float_t x2 = adjusted_x * adjusted_x;
//     float_t x4 = x2 * x2;

//     return adjusted_x * (a_sign * x4 + B * x2 + C);
// }
