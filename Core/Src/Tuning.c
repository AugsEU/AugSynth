#include "Tuning.h"

const float_t TwelveTET[12] = {
    16.3515978312874140f,
    17.3239144360545048f,
    18.3540479948379769f,
    19.4454364826300576f,
    20.6017223070543665f,
    21.8267644645627463f,
    23.1246514194771500f,
    24.4997147488593257f,
    25.9565435987465740f,
    27.5000000000000000f,
    29.1352350948806205f,
    30.8677063285077509f,
};


/// @brief Convert note index to frequency.
float_t NoteToFreq12TET(uint8_t note)
{
    //@TODO: Optimise? Full lookup table?
    uint8_t octave = 0;
    while (note >= 12)
    {
        octave++;
        note -= 12;
    }

    float_t freq = TwelveTET[note];

    while (octave > 0)
    {
        freq *= 2.0f;
        octave--;
    }

    return freq;
}