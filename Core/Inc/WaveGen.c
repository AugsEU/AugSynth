/* ---------------------------------------------------------------------------*/
/* Includes ------------------------------------------------------------------*/
#include "WaveGen.h"
#include "Oscillator.h"
#include "Midi.h"
#include "Tuning.h"
#include "Params.h"
#include "Voice.h"
#include "NLFilter.h"

/* ---------------------------------------------------------------------------*/
/* Constants ------------------------------------------------------------------*/
#define DELAY_BUFFER_LEN 48000
#define DELAY_GLITCH_SIZE 2000
#define LOUDNESS_ALPHA (0.001f)

/* ----------------------------------------------------------------------------*/
/* Globals --------------------------------------------------------------------*/
uint16_t gDelayBuffer[DELAY_BUFFER_LEN];
uint32_t gDelayWriteHead;
int32_t gDelayReadOffset = 0;
int32_t gDelayReadOffsetOffset = 0;

Voice_t gVoiceBank[MIDI_POLYPHONY];
Oscillator_t gLFO;
Oscillator_t gLFOWobbler;
float_t gCurrLoudness = 0.0f;

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

	InitFilter();
	TuningInit();
}



/// @brief Fill sound buffer with number of samples.
/// Note: Buffer has to be twice as long as number of samples.
void FillSoundBuffer(uint16_t* buf, uint16_t samples)
{
	uint16_t pos;
	uint16_t* outp = buf;
	int32_t value, noDelayValue;
	int32_t delayValue;
	uint32_t rndValue = GetNextRand();

	UpdateTuning();

	// Delay
	uint32_t delayMode = EXTRACT_INT_PARAM(gParameters, ASP_DELAY_MODE);
	
	if(delayMode == DELAY_MODE_GLITCH)
	{
		if(rndValue % 13 == 0)
		{
			if(rndValue % 3 == 0 && gDelayReadOffsetOffset < DELAY_GLITCH_SIZE)
			{
				gDelayReadOffsetOffset += DELAY_GLITCH_SIZE / 3;
			}
			else if (rndValue % 3 == 1 && gDelayReadOffsetOffset > -DELAY_GLITCH_SIZE)
			{
				gDelayReadOffsetOffset -= DELAY_GLITCH_SIZE / 3;
			}
		}
	}
	else
	{
		gDelayReadOffsetOffset = 0;
	}

	int32_t delayReadOffset = gParameters[ASP_DELAY_TIME] * DELAY_BUFFER_LEN + gDelayReadOffsetOffset;
	if(delayMode == DELAY_MODE_OFF)
	{
		gDelayReadOffset = 0;
		delayReadOffset = 0;
	}
	else
	{
		if(delayReadOffset < 0) 
		{
			delayReadOffset = 0;
			gDelayReadOffsetOffset = 0;
		}
		else if(delayReadOffset >= DELAY_BUFFER_LEN)
		{
			delayReadOffset =  DELAY_BUFFER_LEN - 1;
			gDelayReadOffsetOffset = 0;
		}
	}
	int32_t delayFeedbackVol = (uint32_t)(gParameters[ASP_DELAY_FEEDBACK] * 32768.0f);
	uint16_t delayGlide = (uint16_t)(gParameters[ASP_DELAY_SHEAR] * (float)AUDIO_BUFF_LEN_DIV4) + 2;

	uint32_t delayReadHead;

	// DCO
	uint32_t waveType1 = EXTRACT_INT_PARAM(gParameters,ASP_DCO_WAVE_TYPE_1);
	uint32_t waveType2 = EXTRACT_INT_PARAM(gParameters,ASP_DCO_WAVE_TYPE_2);
	float_t tune1 = gParameters[ASP_DCO_TUNE_1];
	float_t tune2 = gParameters[ASP_DCO_TUNE_2];
	float_t shape1;// = 1.5f * gParameters[ASP_DCO_WS_1] - 0.25f;
	float_t shape2;// = 1.5f * gParameters[ASP_DCO_WS_2] - 0.25f;
	float_t shape1Lfo = gParameters[ASP_LFO_OSC1_SHAPE];
	float_t shape2Lfo = gParameters[ASP_LFO_OSC2_SHAPE];

	// VCF
	SetFilterType(EXTRACT_INT_PARAM(gParameters, ASP_VCF_MODE));
	float_t filterFreqMod, filterFreq = gParameters[ASP_VCF_CUTOFF];
	float_t filterRes = gParameters[ASP_VCF_RES];
	float_t filterFreqLfo = gParameters[ASP_LFO_VCF_CUTOFF];
	float_t filterResLfo = gParameters[ASP_LFO_VCF_RES];
	float_t filterFollow = gParameters[ASP_VCF_FOLLOW];

	// LFO
	float_t lfoValue;
	uint32_t lfoWaveSelect = EXTRACT_INT_PARAM(gParameters, ASP_LFO_WAVE_TYPE);
	float_t lfoPhaseInc = gParameters[ASP_LFO_RATE];
	float_t lfoWobblePhaseInc = lfoPhaseInc * 0.061804697157f;
	float_t lfoWobble = gParameters[ASP_LFO_WOBBLE];

	// Drive & Gain
	float_t gain = gParameters[ASP_GAIN];
	float_t drive = gParameters[ASP_DRIVE];
	float_t drive_norm = 1.0f /((MIDI_POLYPHONY - 1.0f) * (1.0f - drive) + 1.0f);

	for(int i = 0; i < MIDI_POLYPHONY; i++)
	{
		VoicePrepSampleBlock(&gVoiceBank[i]);
	}

	for (pos = 0; pos < samples; pos++)
	{		
		/*--- LFO ---*/
		OscPhaseInc(&gLFO, lfoPhaseInc * ComputeLfoMult(OscSine(&gLFOWobbler), lfoWobble));
		OscPhaseInc(&gLFOWobbler, lfoWobblePhaseInc);
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
		shape1 = 1.5f * gParameters[ASP_DCO_WS_1] * ComputeLfoMult(lfoValue, shape1Lfo) - 0.25f;
		shape2 = 1.5f * gParameters[ASP_DCO_WS_2] * ComputeLfoMult(lfoValue, shape2Lfo) - 0.25f;

		for(int i = 0; i < MIDI_POLYPHONY; i++)
		{
			y += VoiceGetSample(&gVoiceBank[i], waveType1, waveType2, tune1, tune2, shape1, shape2, lfoValue);
		}

		/*--- Measure loudness ---*/
		float_t sampLoud = fabsf(y) * 2.0f;
		if(sampLoud > 1.0f) sampLoud = 1.0f;
		sampLoud -= 1.0f;
		sampLoud *= sampLoud;
		sampLoud *= sampLoud;
		sampLoud = (1.0f - sampLoud);
		gCurrLoudness = LOUDNESS_ALPHA * sampLoud + (1.0f - LOUDNESS_ALPHA) * gCurrLoudness;

		/*--- Filter ---*/
		filterFreqMod = ComputeLfoMult(lfoValue, filterFreqLfo);
		filterFreqMod *=  ComputeLoudnessMult(gCurrLoudness, filterFollow);
		SetFilterFreq(filterFreq * filterFreqMod);
		SetFilterRes(filterRes * ComputeLfoMult(lfoValue, filterResLfo));
		y = CalcFilterSample(y);

		/*--- Drive & Gain ---*/
		y = drive * DrivenSample(y) + (1.0f - drive) * y;
		y *= drive_norm;
		y *= gain;
		
		/*--- Delay ---*/
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

		noDelayValue = value;
		if(gDelayReadOffset > 0)
		{
			delayReadHead = (gDelayWriteHead + DELAY_BUFFER_LEN - gDelayReadOffset) % DELAY_BUFFER_LEN;
			delayValue = (int16_t)gDelayBuffer[delayReadHead];
			value += delayValue;
		}

		/*--- Write to buffer ---*/
		if (value < -32768)
		{
			value = -32768;
		}
		else if (value > 32767)
		{
			value = 32767;
		}

		*outp++ = (int16_t)value;
		*outp++ = (int16_t)value;

		/*--- Delay write ---*/
		if (delayMode == DELAY_MODE_SLAPBACK)
		{
			value = noDelayValue; // Just write delay without feedback
		}

		value *= delayFeedbackVol;
		value /= 32768;

		gDelayBuffer[gDelayWriteHead] = (uint16_t)(int16_t)value;
		gDelayWriteHead = (gDelayWriteHead + 1) % DELAY_BUFFER_LEN;
	}
}