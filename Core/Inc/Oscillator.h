#ifndef INC_OSCILLATOR_H_
#define INC_OSCILLATOR_H_
#include <math.h>
#include <stdint.h>
#include <stdbool.h>
#include "CONSTANTS.h"

typedef struct
{
	float_t 	mVol;
	float_t 	mPhaseInc;	// Number to increment based on freq.
	float_t 	mPhase;	// Radians
} Oscillator_t;

void OscInit(Oscillator_t* osc, float_t volume);
void OscFreqSet(Oscillator_t* osc, float_t hertz);
void OscPhaseInc(Oscillator_t* osc);

float_t OscSawTooth(Oscillator_t* osc);
float_t OscSine(Oscillator_t* osc);

#endif /* INC_OSCILLATOR_H_ */
