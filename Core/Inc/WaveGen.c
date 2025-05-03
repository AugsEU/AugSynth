/* ---------------------------------------------------------------------------*/
/* Includes ------------------------------------------------------------------*/
#include "WaveGen.h"
#include "Oscillator.h"
#include "Midi.h"
#include "Tuning.h"
#include "Params.h"
#include "Voice.h"

/* ---------------------------------------------------------------------------*/
/* Constants ------------------------------------------------------------------*/
#define DELAY_BUFFER_LEN 48000

const float_t DRIVE_K = 1.0f;
const float_t DRIVE_ALPHA = 4.0f * DRIVE_K + 1.0f;

/* ----------------------------------------------------------------------------*/
/* Globals --------------------------------------------------------------------*/
uint16_t gDelayBuffer[DELAY_BUFFER_LEN];
uint32_t gDelayWriteHead;
int32_t gDelayReadOffset = 0;

Voice_t gVoiceBank[MIDI_POLYPHONY];
Oscillator_t gLFO;

extern float gParameters[128];

/* ----------------------------------------------------------------------------*/
/* Public functions -----------------------------------------------------------*/

/// @brief Initialise synth.
void SynthInit(void)
{
	for(int i = 0; i < MIDI_POLYPHONY; i++)
	{
		VoiceInit(&gVoiceBank[i]);
	}

	gDelayWriteHead = 0;
	for(int i = 0; i < DELAY_BUFFER_LEN; i++)
	{
		gDelayBuffer[i] = 0;
	}

	OscInit(&gLFO);

	gParameters[ASP_TUNING]	 			= 0.0f;
	gParameters[ASP_DRIVE]	 			= 0.0f;
	gParameters[ASP_GAIN]	 			= 0.5f;
	gParameters[ASP_DCO_WAVE_SHAPE_1]	= 0.2f;
	gParameters[ASP_DCO_TUNE_1]	 		= 0.2f;
	gParameters[ASP_DCO_VOL_1]	 		= 1.0f;
	gParameters[ASP_DCO_WAVE_SHAPE_2]	= 1.0f;
	gParameters[ASP_DCO_TUNE_2]	 		= 0.5f;
	gParameters[ASP_DCO_VOL_2]	 		= 0.8f;
	gParameters[ASP_VCF_CUTOFF]	 		= 0.0f;
	gParameters[ASP_VCF_RES]	 		= 0.0f;
	gParameters[ASP_VCF_MODE]	 		= 0.0f;
	gParameters[ASP_VCF_CUTOFF_LFO]	 	= 0.0f;
	gParameters[ASP_VCF_RES_LFO]	 	= 0.0f;
	gParameters[ASP_VCF_ENV1]	 		= 0.0f;
	gParameters[ASP_LFO_RATE]	 		= 0.0f;
	gParameters[ASP_LFO_WAVE_SHAPE]	 	= 0.0f;
	gParameters[ASP_LFO_ATTACK]	 		= 0.0f;
	gParameters[ASP_LFO_GAIN]	 		= 0.0f;
	gParameters[ASP_LFO_OSC1_TUNE]	 	= 0.0f;
	gParameters[ASP_LFO_OSC2_TUNE]	 	= 0.0f;
	gParameters[ASP_ENV_ATTACK1]	 	= 1.0f;
	gParameters[ASP_ENV_SUSTAIN1]	 	= 1.0f;
	gParameters[ASP_ENV_DECAY1]	 		= 0.8f;
	gParameters[ASP_ENV_ATTACK2]	 	= 1.0f;
	gParameters[ASP_ENV_SUSTAIN2]	 	= 1.0f;
	gParameters[ASP_ENV_DECAY2] 		= 0.8f;
}



/// @brief Fill sound buffer with number of samples.
/// Note: Buffer has to be twice as long as number of samples.
void FillSoundBuffer(uint16_t* buf, uint16_t samples)
{
	uint16_t pos;
	uint16_t* outp = buf+1;
	int32_t value;
	int32_t delayValue;

	// Delay
	int32_t delayReadOffset = gParameters[ASP_DELAY_TIME] * DELAY_BUFFER_LEN;
	if(gDelayReadOffset < delayReadOffset)
	{
		gDelayReadOffset += 2;
	}
	else if (gDelayReadOffset > delayReadOffset)
	{
		gDelayReadOffset -= 2;
	}
	int32_t delayFeedbackVol = (uint32_t)(gParameters[ASP_DELAY_FEEDBACK] * 32768.0f);
	uint32_t delayReadHead;


	// DCO
	float_t waveShape1 = gParameters[ASP_DCO_WAVE_SHAPE_1];
	float_t waveShape2 = gParameters[ASP_DCO_WAVE_SHAPE_2];
	float_t tune1 = powf(2, floorf(4.0f * gParameters[ASP_DCO_TUNE_1] - 1.5f));
	float_t tune2 = powf(2, floorf(4.0f * gParameters[ASP_DCO_TUNE_2] - 1.5f));

	// LFO
	float_t lfoValue;
	float_t lfoWaveShape = gParameters[ASP_LFO_WAVE_SHAPE];
	float_t lfoGain = gParameters[ASP_LFO_GAIN] * 0.5f;
	float_t lfoPhaseInc = gParameters[ASP_LFO_RATE];
	lfoPhaseInc *= lfoPhaseInc * lfoPhaseInc;
	lfoPhaseInc *= 35.0f;
	lfoPhaseInc += 0.1f;
	lfoPhaseInc *= SAMPLE_PERIOD;

	// Drive & Gain
	float_t gain = (2.0f / (float_t)MIDI_POLYPHONY) * gParameters[ASP_GAIN];
	float_t drive = 1.0f + gParameters[ASP_DRIVE] * (DRIVE_ALPHA-1.0f);
	float_t da = DRIVE_K * (drive - 1.0f);
	float_t db = (1.0f - drive - da);

	for(int i = 0; i < MIDI_POLYPHONY; i++)
	{
		VoicePrepSampleBlock(&gVoiceBank[i]);
	}

	for (pos = 0; pos < samples; pos++)
	{
		/*--- LFO ---*/
		OscPhaseInc(&gLFO, lfoPhaseInc);
		lfoValue = OscSine(&gLFO); //@TODO implement different wave shapes

		/*--- Generate waveform ---*/
		float_t	y = 0.0f;

		for(int i = 0; i < MIDI_POLYPHONY; i++)
		{
			y += VoiceGetSample(&gVoiceBank[i], waveShape1, waveShape2, tune1, tune2, lfoValue, lfoGain);
		}

		/*--- Drive & Gain ---*/
		y *= gain;

		if (y < 0.0f)
		{
			y *= -1.0f;
			y *= (da * y * y + db * y + drive); // Guaranteed to still be -1 to 1
			y *= -1.0f;
		}
		else
		{
			y *= (da * y * y + db * y + drive);
		}

		value = (int32_t)((32767.0f) * y);

		// Delay read
		if(gDelayReadOffset > 0)
		{
			delayReadHead = (gDelayWriteHead + DELAY_BUFFER_LEN - gDelayReadOffset) % DELAY_BUFFER_LEN;
			delayValue = (int16_t)gDelayBuffer[delayReadHead];
			value += delayValue;
		}

		if (value <= -32768)
		{
			value = -32768;
		}
		else if (value >= 32767)
		{
			value = 32767;
		}

		*outp = (int16_t)value;
		outp += 2;

		// Delay write
		value *= delayFeedbackVol;
		value /= 32768;

		gDelayBuffer[gDelayWriteHead] = (uint16_t)(int16_t)value;
		gDelayWriteHead = (gDelayWriteHead + 1) % DELAY_BUFFER_LEN;
	}
}