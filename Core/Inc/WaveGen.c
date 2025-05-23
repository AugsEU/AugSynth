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
Oscillator_t gLFOWobbler;
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
	OscInit(&gLFOWobbler);

	for(int i = 0; i < ASP_NUM_PARAMS; i++)
	{
		gParameters[i] = 0.0f;
	}

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
	uint32_t waveType1 = EXTRACT_INT_PARAM(gParameters,ASP_DCO_WAVE_TYPE_1);
	uint32_t waveType2 = EXTRACT_INT_PARAM(gParameters,ASP_DCO_WAVE_TYPE_2);
	float_t tune1 = gParameters[ASP_DCO_TUNE_1];
	float_t tune2 = gParameters[ASP_DCO_TUNE_2];
	float_t shape1 = 1.5f * gParameters[ASP_DCO_WS_1] - 0.25f;
	float_t shape2 = 1.5f * gParameters[ASP_DCO_WS_2] - 0.25f;

	// VCF
	// float_t filterMode = gParameters[ASP_VCF_MODE];
	// float_t filterFreqMod, filterFreq = gParameters[ASP_VCF_CUTOFF];
	// float_t filterResMod, filterRes = gParameters[ASP_VCF_RES];
	// float_t filterFreqLfo = gParameters[ASP_VCF_CUTOFF_LFO];
	// float_t filterResLfo = gParameters[ASP_VCF_RES_LFO];
	// float_t filterEnvFollow = gParameters[ASP_VCF_CUTOFF];

	// LFO
	float_t lfoValue;
	uint32_t lfoWaveSelect = EXTRACT_INT_PARAM(gParameters, ASP_LFO_WAVE_TYPE);
	float_t lfoPhaseInc = gParameters[ASP_LFO_RATE];
	float_t lfoWobblePhaseInc = lfoPhaseInc;
	float_t lfoWobble = gParameters[ASP_LFO_WOBBLE];

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
			lfoValue = OscSquareLF(&gLFO);
			break;
		case OSC_MODE_SAW:
			lfoValue = OscSawTooth(&gLFO, lfoPhaseInc);
			break;
		}

		/*--- Generate waveform ---*/
		float_t	y = 0.0f;

		for(int i = 0; i < MIDI_POLYPHONY; i++)
		{
			y += VoiceGetSample(&gVoiceBank[i], waveType1, waveType2, tune1, tune2, shape1, shape2, lfoValue, 0.0f);
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