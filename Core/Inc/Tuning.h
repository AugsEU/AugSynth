#ifndef TUNING_H
#define TUNING_H

#include <math.h>
#include <stdint.h>

void TuningInit();
void UpdateTuning();
float_t NoteToFreq(uint8_t note);

#endif // TUNING_H