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
const float_t DRIVE_K = 1.0f;
const float_t DRIVE_ALPHA = 4.0f * DRIVE_K + 1.0f;

/* ----------------------------------------------------------------------------*/
/* Globals --------------------------------------------------------------------*/
Voice_t gVoiceBank[MIDI_POLYPHONY];

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

	gParameters[ASP_TUNING]	 			= 0.0f;
	gParameters[ASP_DRIVE]	 			= 0.0f;
	gParameters[ASP_GAIN]	 			= 0.5f;
	gParameters[ASP_DCO_WAVE_SHAPE_1]	= 0.0f;
	gParameters[ASP_DCO_TUNE_1]	 		= 0.2f;
	gParameters[ASP_DCO_VOL_1]	 		= 1.0f;
	gParameters[ASP_DCO_WAVE_SHAPE_2]	= 1.0f;
	gParameters[ASP_DCO_TUNE_2]	 		= 0.5f;
	gParameters[ASP_DCO_VOL_2]	 		= 1.0f;
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
	gParameters[ASP_ENV_DECAY1]	 		= 0.5f;
	gParameters[ASP_ENV_ATTACK2]	 	= 1.0f;
	gParameters[ASP_ENV_SUSTAIN2]	 	= 1.0f;
	gParameters[ASP_ENV_DECAY2] 		= 0.5f;
}



/// @brief Fill sound buffer with number of samples.
/// Note: Buffer has to be twice as long as number of samples.
void FillSoundBuffer(uint16_t* buf, uint16_t samples)
{
	uint16_t pos;
	uint16_t* outp = buf;
	uint16_t value;

	float_t waveShape1 = gParameters[ASP_DCO_WAVE_SHAPE_1];
	float_t waveShape2 = gParameters[ASP_DCO_WAVE_SHAPE_2];
	float_t tune1 = powf(2, floorf(4.0f * gParameters[ASP_DCO_TUNE_1] - 1.5f));
	float_t tune2 = powf(2, floorf(4.0f * gParameters[ASP_DCO_TUNE_2] - 1.5f));

	float_t gain = (4.0f / (float_t)MIDI_POLYPHONY) * gParameters[ASP_GAIN];
	float_t drive = 1.0f + gParameters[ASP_DRIVE] * (DRIVE_ALPHA-1.0f);

	float_t da = DRIVE_K * (drive - 1.0f);
	float_t db = (1.0f - drive - da);

	for(int i = 0; i < MIDI_POLYPHONY; i++)
	{
		VoicePrepSampleBlock(&gVoiceBank[i]);
	}

	for (pos = 0; pos < samples; pos++)
	{
		/*--- Generate waveform ---*/
		float_t	y = 0.0f;

		for(int i = 0; i < MIDI_POLYPHONY; i++)
		{
			y += VoiceGetSample(&gVoiceBank[i], waveShape1, waveShape2, tune1, tune2);
		}

		/*--- clipping ---*/
		y *= gain;

		y = (y > 1.0f) ? 1.0f : y; //Clip
		y = (y < -1.0f) ? -1.0f : y;

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
		

		value = (uint16_t)((int16_t)((32767.0f) * y)); 

		*outp++ = value;
		*outp++ = value;
	}
}