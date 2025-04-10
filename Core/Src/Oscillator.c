#include "Oscillator.h"
#include "Constants.h"

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
    osc->mPhaseInc = A_2PI * SAMPLE_PERIOD * hertz;
}



/// @brief Increment phase of oscillator by 1 sample.
void OscPhaseInc(Oscillator_t* osc)
{
    osc->mPhase += osc->mPhaseInc;

	if (osc->mPhase >= A_2PI)
    {
		osc->mPhase -= A_2PI;
    }
}




/// @brief Get value of oscillator as saw tooth.
float_t OscSawTooth(float_t phase)
{
    float_t value = (1.0f / A_PI) * phase;
	if (phase >= A_PI)
    {
        value -= 2.0f;
    }

    return value;
}