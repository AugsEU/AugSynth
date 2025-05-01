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
void OscInit(Oscillator_t* osc, float_t volume)
{
    osc->mVol = volume;
    osc->mPhaseInc = 0.0f;
    osc->mPhase = 0.0f;
}



/// @brief Set oscillator frequency.
void OscFreqSet(Oscillator_t* osc, float_t hertz)
{
    osc->mPhaseInc = SAMPLE_PERIOD * hertz;
}



/// @brief Increment phase of oscillator by 1 sample.
void OscPhaseInc(Oscillator_t* osc)
{
    osc->mPhase += osc->mPhaseInc;

	if (osc->mPhase >= 1.0f)
    {
		osc->mPhase -= 1.0f;
    }
}




/// @brief Get value of oscillator as saw tooth.
float_t OscSawTooth(Oscillator_t* osc)
{
    float_t phase = osc->mPhase;
    float_t dt = osc->mPhaseInc;

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
    float_t value = osc->mPhase;
    
    if (osc->mPhase > 0.5f)
    {
        value -= 0.5f;
        value *= (16.0f * value - 8.0f);
    }
    else
    {
        value *= (8.0f - 16.0f * value);
    }

    return value;
}