#ifndef INC_OSCILLATOR_H_
#define INC_OSCILLATOR_H_
#include <math.h>
#include <stdint.h>
#include <stdbool.h>
#include "CONSTANTS.h"

typedef struct
{
	uint32_t 	mVol;
	uint32_t 	mPhaseInc;	// Number to increment based on freq.
	uint32_t 	mPhase;	// Radians
} Oscillator_t;

void OscInit(Oscillator_t* osc);
void OscFreqSet(Oscillator_t* osc, float_t hertz);
void OscVolumeSet(Oscillator_t* osc, float_t volume);
void OscPhaseInc(Oscillator_t* osc);

uint32_t OscSawTooth(uint32_t phase);

#endif /* INC_OSCILLATOR_H_ */
