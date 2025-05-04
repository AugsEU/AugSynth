#ifndef __MIDI_H
#define __MIDI_H

/* ----------------------------------------------------------------------------*/
/* Includes -------------------------------------------------------------------*/
#include <stdint.h>
#include <math.h>



/* ----------------------------------------------------------------------------*/
/* Constants ------------------------------------------------------------------*/
#define MIDI_NOTE_ON 0xC0
#define MIDI_NOTE_OFF 0x80
#define MIDI_PARAM_NUM_MASK 0x7F

#define MIDI_POLYPHONY 7



/* ----------------------------------------------------------------------------*/
/* Types ----------------------------------------------------------------------*/
typedef struct
{
    uint8_t mNoteIdx;
    uint8_t mNoteVelocity;
} MidiNote_t;



/* ----------------------------------------------------------------------------*/
/* Public functions -----------------------------------------------------------*/
void MidiInit(void);
void ProcessMidiMessage(uint8_t* buff);

float_t VelocityToVolume(uint8_t velocity);

#endif // __MIDI_H