#include "Oscillator.h"
#include "Constants.h"

/// @brief Initialise an oscillator 
void OscInit(Oscillator_t* osc)
{
    osc->mVol = 0;
    osc->mPhaseInc = 0;
    osc->mPhase = 0;
}



/// @brief Set oscillator frequency.
void OscFreqSet(Oscillator_t* osc, float_t hertz)
{
    osc->mPhaseInc = ((uint16_t)hertz << 16) / SAMPLE_RATE;
}



/// @brief Set osc volume
void OscVolumeSet(Oscillator_t* osc, float_t volume)
{
    osc->mVol = (uint32_t)(65536.0f * volume) & 0xFFFF;
}



/// @brief Increment phase of oscillator by 1 sample.
void OscPhaseInc(Oscillator_t* osc)
{
    osc->mPhase += osc->mPhaseInc;
    osc->mPhase &= 0xFFFF; // Wrap around.
}




/// @brief Get value of oscillator as saw tooth.
uint32_t OscSawTooth(uint32_t phase)
{
    return ((phase+32768) << 1) & 0xFFFF;
}