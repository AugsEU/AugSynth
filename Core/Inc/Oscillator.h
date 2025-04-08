#ifndef INC_OSCILLATOR_H_
#define INC_OSCILLATOR_H_
#include <math.h>
#include <stdint.h>
#include <stdbool.h>
#include "CONSTANTS.h"

typedef struct
{
	float_t 	mVol;
	float_t 	mFreq;	// Hertz
	float_t 	mPhase;	// Radians
	float_t 	mOut;	// Output sample
} Oscillator_t;

void OscInit(Oscillator_t* osc, float_t volume);
void OscPhaseInc(Oscillator_t* osc);
float_t OscSawTooth(Oscillator_t* osc);

#endif /* INC_OSCILLATOR_H_ */
