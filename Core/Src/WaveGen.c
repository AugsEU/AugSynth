/* ---------------------------------------------------------------------------*/
/* Includes ------------------------------------------------------------------*/
#include "WaveGen.h"
#include "Oscillator.h"
#include "Midi.h"
#include "Tuning.h"

/* ----------------------------------------------------------------------------*/
/* Globals --------------------------------------------------------------------*/
Oscillator_t gOscBank[MIDI_POLYPHONY];

extern MidiNote_t gPlayingNotes[MIDI_POLYPHONY];


/* ----------------------------------------------------------------------------*/
/* Public functions -----------------------------------------------------------*/

/// @brief Initialise synth.
void SynthInit(void)
{
	for(int i = 0; i < MIDI_POLYPHONY; i++)
	{
		OscInit(&gOscBank[i], 0.0f);
	}
}



/// @brief Fill sound buffer with number of samples.
/// Note: Buffer has to be twice as long as number of samples.
void FillSoundBuffer(uint16_t* buf, uint16_t samples)
{
	uint16_t pos;
	uint16_t* outp = buf;
	uint16_t value;

	for(int i = 0; i < MIDI_POLYPHONY; i++)
	{
		uint8_t velocity = gPlayingNotes[i].mNoteVelocity;
		uint8_t noteIdx = gPlayingNotes[i].mNoteIdx;
		
		OscFreqSet(&gOscBank[i], NoteToFreq12TET(noteIdx));
		gOscBank[i].mVol = VelocityToVolume(velocity);
	}

	for (pos = 0; pos < samples; pos++)
	{
		/*--- Generate waveform ---*/
		float_t	y = 0.0f;
		float_t dy;

		for(int i = 0; i < MIDI_POLYPHONY; i++)
		{
			OscPhaseInc(&gOscBank[i]);
			dy = OscSine(&gOscBank[i]);
			dy *= gOscBank[i].mVol;
			y += dy;
		}

		/*--- clipping ---*/
		y = (y > 1.0f) ? 1.0f : y; //clip too loud left samples
		y = (y < -1.0f) ? -1.0f : y;

		value = (uint16_t)((int16_t)((32767.0f) * y)); 

		*outp++ = value;
		*outp++ = value;
	}
}