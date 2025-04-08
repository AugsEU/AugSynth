#ifndef INC_WAVEGEN_H_
#define INC_WAVEGEN_H_

#include <math.h>
#include <stdint.h>
#include <stdbool.h>
#include "main.h"

void SynthInit(void);
void FillSoundBuffer(uint16_t* buf, uint16_t samples);

#endif /* INC_WAVEGEN_H_ */