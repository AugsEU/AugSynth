#ifndef __MIDI_H
#define __MIDI_H

/* ----------------------------------------------------------------------------*/
/* Includes -------------------------------------------------------------------*/
#include <stdint.h>
#include <math.h>



/* ----------------------------------------------------------------------------*/
/* Constants ------------------------------------------------------------------*/
#define MIDI_CMD_MSG 0x80
#define MIDI_CMD_MASK 0xF0
#define MIDI_CMD_SYS_EX 0xF0
#define MIDI_CMD_NOTE_ON 0x90
#define MIDI_CMD_NOTE_OFF 0x80
#define MIDI_CMD_CHANNEL 0x0F

#define MIDI_CHANNEL 0x00 // Midi channel we are listening to
#define MIDI_POLYPHONY 70
#define MIDI_NUM_NOTES 127



/* ----------------------------------------------------------------------------*/
/* Types ----------------------------------------------------------------------*/
typedef struct
{
    uint8_t mNoteIdx;
    uint8_t mNoteVelocity;
    uint8_t mTimeStamp;
} MidiNote_t;



/* ----------------------------------------------------------------------------*/
/* Public functions -----------------------------------------------------------*/
void MidiInit(void);
void ProcessMidiMessage(uint8_t* buff);

float_t VelocityToVolume(uint8_t velocity);

#endif // __MIDI_H