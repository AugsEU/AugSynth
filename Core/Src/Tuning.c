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

const float_t TwelveJC[12] = {
    16.3515978312f,
    17.4417043534f,
    18.3955475602f,
    19.6219173975f,
    20.4394972891f,
    21.8021304417f,
    23.2556058045f,
    24.5273967469f,
    26.1625565301f,
    27.2526630521f,
    29.0695072556f,
    30.6592459337f,
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


/// @brief Convert note index to frequency.
float_t NoteToFreq12JC(uint8_t note)
{
    //@TODO: Optimise? Full lookup table?
    uint8_t octave = 0;
    while (note >= 12)
    {
        octave++;
        note -= 12;
    }

    float_t freq = TwelveJC[note];

    while (octave > 0)
    {
        freq *= 2.0f;
        octave--;
    }

    return freq;
}