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
		OscInit(&gOscBank[i]);
	}
}



/// @brief Fill sound buffer with number of samples.
/// Note: Buffer has to be twice as long as number of samples.
void FillSoundBuffer(uint16_t* buf, uint16_t samples)
{
	uint16_t pos;
	uint16_t* outp = buf;

	for(int i = 0; i < MIDI_POLYPHONY; i++)
	{
		uint8_t velocity = gPlayingNotes[i].mNoteVelocity;
		uint8_t noteIdx = gPlayingNotes[i].mNoteIdx;
		
		OscFreqSet(&gOscBank[i], NoteToFreq12JC(noteIdx));
		OscVolumeSet(&gOscBank[i], VelocityToVolume(velocity));
	}

	for (pos = 0; pos < samples; pos++)
	{
		/*--- Generate waveform ---*/
		uint32_t y = 0;

		for(int i = 0; i < MIDI_POLYPHONY; i++)
		{
			OscPhaseInc(&gOscBank[i]);
			uint32_t temp = OscSawTooth(gOscBank[i].mPhase);
			temp *= gOscBank[i].mVol;
			y += temp >> 16;
		}

		uint16_t value = (uint32_t)(y & 0xFFFF);

		*outp++ = value;
		*outp++ = value;
	}
}