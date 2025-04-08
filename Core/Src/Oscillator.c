#include "Oscillator.h"
#include "Constants.h"

/// @brief Initialise an oscillator 
void OscInit(Oscillator_t* osc, float_t volume)
{
    osc->mVol = volume;
    osc->mFreq = 0.0f;
    osc->mPhase = 0.0f;
    osc->mOut = 0.0f;
}



/// @brief Increment phase of oscillator by 1 sample.
void OscPhaseInc(Oscillator_t* osc)
{
    osc->mPhase += A_2PI * SAMPLE_PERIOD * osc->mFreq;

	if (osc->mPhase >= A_2PI)
    {
		osc->mPhase -= A_2PI;
    }
}



/// @brief Get value of oscillator as saw tooth.
float_t OscSawTooth(Oscillator_t* osc)
{
    float_t value = (1.0f / A_PI) * osc->mPhase;
	if (osc->mPhase >= A_PI)
    {
        value -= 2.0f;
    }

    value *= osc->mVol;

    return value;
}