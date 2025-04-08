/* ---------------------------------------------------------------------------*/
/* Includes ------------------------------------------------------------------*/
#include "WaveGen.h"
#include "Oscillator.h"

/* ----------------------------------------------------------------------------*/
/* Globals --------------------------------------------------------------------*/
Oscillator_t gOp1;

/* ----------------------------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/

/* ----------------------------------------------------------------------------*/
/* Private user code ---------------------------------------------------------*/

/// @brief Initialise synth.
void SynthInit(void)
{
    OscInit(&gOp1, 0.01f);
    gOp1.mFreq = 200.0f;
}



/// @brief Fill sound buffer with number of samples.
/// Note: Buffer has to be twice as long as number of samples.
void FillSoundBuffer(uint16_t* buf, uint16_t samples)
{
	uint16_t 	pos;
	uint16_t 	*outp;
	float	 	y = 0;
	float	 	yL, yR ;
	uint16_t 	valueL, valueR;

	outp = buf;

	for (pos = 0; pos < samples; pos++)
	{
		/*--- Generate waveform ---*/
        OscPhaseInc(&gOp1);
		y = OscSawTooth(&gOp1);

		//creates a copy of y for left and right output channels
		yL = yR = y;

		/*--- clipping ---*/
		yL = (yL > 1.0f) ? 1.0f : yL; //clip too loud left samples
		yL = (yL < -1.0f) ? -1.0f : yL;

		yR = (yR > 1.0f) ? 1.0f : yR; //clip too loud right samples
		yR = (yR < -1.0f) ? -1.0f : yR;

		/****** let's hear the new sample *******/

		valueL = (uint16_t)((int16_t)((32767.0f) * yL)); // conversion float -> int
		valueR = (uint16_t)((int16_t)((32767.0f) * yR));

		*outp++ = valueL; // left channel sample
		*outp++ = valueR; // right channel sample
	}
}