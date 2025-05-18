/* ---------------------------------------------------------------------------*/
/* Includes ------------------------------------------------------------------*/
#include "WaveGen.h"
#include "Oscillator.h"
#include "Midi.h"
#include "Tuning.h"
#include "Params.h"
#include "Voice.h"
#include "SVFilter.h"

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
SVFilter_t gFilter;

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
	gParameters[ASP_GAIN]	 			= 0.1f;
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
	gParameters[ASP_VCF_FOLLOW]	 		= 0.0f;
	gParameters[ASP_LFO_RATE]	 		= 0.5f;
	gParameters[ASP_LFO_WAVE_SHAPE]	 	= 0.0f;
	gParameters[ASP_LFO_ATTACK]	 		= 1.0f;
	gParameters[ASP_LFO_WOBBLE]	 		= 0.0f;
	gParameters[ASP_LFO_OSC1_TUNE]	 	= 0.0f;
	gParameters[ASP_LFO_OSC2_TUNE]	 	= 0.0f;
	gParameters[ASP_ENV_ATTACK1]	 	= 1.0f;
	gParameters[ASP_ENV_SUSTAIN1]	 	= 1.0f;
	gParameters[ASP_ENV_DECAY1]	 		= 0.8f;
	gParameters[ASP_ENV_ATTACK2]	 	= 1.0f;
	gParameters[ASP_ENV_SUSTAIN2]	 	= 1.0f;
	gParameters[ASP_ENV_DECAY2] 		= 0.8f;

	SvfInit(&gFilter);
}



/// @brief Fill sound buffer with number of samples.
/// Note: Buffer has to be twice as long as number of samples.
void FillSoundBuffer(uint16_t* buf, uint16_t samples)
{
	uint16_t pos;
	uint16_t* outp = buf;
	int32_t value;
	int32_t delayValue;

	// Delay
	int32_t delayReadOffset = gParameters[ASP_DELAY_TIME] * DELAY_BUFFER_LEN;
	int32_t delayFeedbackVol = (uint32_t)(gParameters[ASP_DELAY_FEEDBACK] * 32768.0f);
	uint16_t delayGlide = (uint16_t)(gParameters[ASP_DELAY_SHEAR] * (float)AUDIO_BUFF_LEN_DIV4) + 2;
	uint32_t delayReadHead;

	// DCO
	uint32_t waveShape1 = EXTRACT_INT_PARAM(gParameters,ASP_DCO_WAVE_SHAPE_1);
	uint32_t waveShape2 = EXTRACT_INT_PARAM(gParameters,ASP_DCO_WAVE_SHAPE_2);
	float_t tune1 = gParameters[ASP_DCO_TUNE_1];
	float_t tune2 = gParameters[ASP_DCO_TUNE_2];

	// VCF
	float_t filterMode = gParameters[ASP_VCF_MODE];
	float_t filterFreqMod, filterFreq = gParameters[ASP_VCF_CUTOFF];
	float_t filterResMod, filterRes = gParameters[ASP_VCF_RES];
	float_t filterFreqLfo = gParameters[ASP_VCF_CUTOFF_LFO];
	float_t filterResLfo = gParameters[ASP_VCF_RES_LFO];
	float_t filterEnvFollow = gParameters[ASP_VCF_CUTOFF];

	// LFO
	float_t lfoValue;
	uint32_t lfoWaveSelect = EXTRACT_INT_PARAM(gParameters, ASP_LFO_WAVE_SHAPE);

	float_t lfoGain = 0.0f;//gParameters[ASP_] * 0.5f;
	float_t lfoPhaseInc = gParameters[ASP_LFO_RATE];

	// Drive & Gain
	float_t gain = gParameters[ASP_GAIN];
	float_t drive = gParameters[ASP_DRIVE];
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
		switch (lfoWaveSelect)
		{
		default:
		case OSC_MODE_SINE:
			lfoValue = OscSine(&gLFO);
			break;
		case OSC_MODE_SQUARE:
			lfoValue = OscSquare(&gLFO, lfoPhaseInc);
			break;
		case OSC_MODE_SAW:
			lfoValue = OscSawTooth(&gLFO, lfoPhaseInc);
			break;
		}

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

		//y = SvfProcess(&gFilter, y, filterFreq * (1.0f - filterFreqLfo * (lfoValue + 1.0f)), filterRes * (1.0f - filterResLfo * (lfoValue + 1.0f)), filterMode);
		value = (int32_t)((32767.0f) * y);

		// Delay read
		if ((pos % delayGlide) == 0)
		{
			if(gDelayReadOffset < delayReadOffset)
			{
				gDelayReadOffset += 1;
			}
			else if (gDelayReadOffset > delayReadOffset)
			{
				gDelayReadOffset -= 1;
			}
		}

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

		*outp++ = (int16_t)value;
		*outp++ = (int16_t)value;

		// Delay write
		value *= delayFeedbackVol;
		value /= 32768;

		gDelayBuffer[gDelayWriteHead] = (uint16_t)(int16_t)value;
		gDelayWriteHead = (gDelayWriteHead + 1) % DELAY_BUFFER_LEN;
	}
}