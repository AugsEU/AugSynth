#ifndef INC_OSCILLATOR_H_
#define INC_OSCILLATOR_H_
#include <math.h>
#include <stdint.h>
#include <stdbool.h>
#include "CONSTANTS.h"

typedef struct
{
	float_t 	mPhase;	// Radians
} Oscillator_t;

void OscInit(Oscillator_t* osc);
void OscPhaseInc(Oscillator_t* osc, float_t phaseInc);

float_t OscSawTooth(Oscillator_t* osc, float_t phaseInc);
float_t OscSine(Oscillator_t* osc);

#endif /* INC_OSCILLATOR_H_ */
