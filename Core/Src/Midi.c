#include "Midi.h"
#include "Constants.h"

/* ----------------------------------------------------------------------------*/
/* Global variables -----------------------------------------------------------*/
MidiNote_t gPlayingNotes[MIDI_POLYPHONY];
uint8_t gNoteTimeIdx = 0;


void NoteOn(uint8_t noteIdx, uint8_t velocity);
void NoteOff(uint8_t noteIdx);

/* ----------------------------------------------------------------------------*/
/* Public functions -----------------------------------------------------------*/

/// @brief Initialise midi
void MidiInit(void)
{
    for (int i = 0; i < MIDI_POLYPHONY; i++)
    {
        gPlayingNotes[i].mNoteIdx = 0;
        gPlayingNotes[i].mNoteVelocity = 0;
        gPlayingNotes[i].mTimeStamp = 0;
    }

    gNoteTimeIdx = 0;
}



/// @brief Process a midi message.
void ProcessMidiMessage(uint8_t* buff)
{
    uint8_t midiCmd = buff[0] & MIDI_CMD_MASK;
    uint8_t midiDataByte0 = buff[1];
    uint8_t midiDataByte1 = buff[2];

    if (midiCmd == MIDI_CMD_NOTE_ON)
    {
        NoteOn(midiDataByte0, midiDataByte1);
    }
    else if(midiCmd == MIDI_CMD_NOTE_OFF)
    {
        NoteOff(midiDataByte0);
    }
}


/// @brief Convert velocity to volume
float_t VelocityToVolume(uint8_t velocity)
{
    return (0.02f) * (1.0f / 127.0f) * (float_t)velocity; 
}

/* ----------------------------------------------------------------------------*/
/* Private functions ----------------------------------------------------------*/

/// @brief Turns note on.
void NoteOn(uint8_t noteIdx, uint8_t velocity)
{
    //@TODO: Voice stealing.
    for (int i = 0; i < MIDI_POLYPHONY; i++)
    {
        if (gPlayingNotes[i].mNoteVelocity == 0)
        {
            gPlayingNotes[i].mNoteIdx = noteIdx;
            gPlayingNotes[i].mNoteVelocity = velocity;
            gPlayingNotes[i].mTimeStamp = gNoteTimeIdx++;
            break;
        }
    }
}

/// @brief Turns note off.
void NoteOff(uint8_t noteIdx)
{
    for (int i = 0; i < MIDI_POLYPHONY; i++)
    {
        if (gPlayingNotes[i].mNoteIdx == noteIdx)
        {
            gPlayingNotes[i].mNoteVelocity = 0;
            break;
        }
    }
}