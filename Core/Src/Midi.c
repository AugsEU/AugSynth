#include "Midi.h"
#include "Constants.h"

/* ----------------------------------------------------------------------------*/
/* Global variables -----------------------------------------------------------*/
MidiNote_t gPlayingNotes[MIDI_POLYPHONY];
float gParameters[128];

void NoteOn(uint8_t noteIdx, uint8_t velocity);
void NoteOff(uint8_t noteIdx);

/* ----------------------------------------------------------------------------*/
/* Public functions -----------------------------------------------------------*/

/// @brief Initialise midi
void MidiInit(void)
{
    for (int i = 0; i < MIDI_POLYPHONY; i++)
    {
        gPlayingNotes[i].mNoteIdx = 0xFF;
        gPlayingNotes[i].mNoteVelocity = 0;
        gPlayingNotes[i].mTimeStamp = 0;
    }
}



/// @brief Process a midi message.
void ProcessMidiMessage(uint8_t* buff)
{
    uint8_t header = buff[0];
    float paramValue;

    switch (header)
    {
    case MIDI_NOTE_ON:
        NoteOn(buff[1], buff[2]);
        break;
    case MIDI_NOTE_OFF:
        NoteOff(buff[1]);
        break;
    default: // Param change
        paramValue = *(float*)(buff+1);
        gParameters[header & MIDI_PARAM_NUM_MASK] = paramValue;
        break;
    }
}


/// @brief Convert velocity to volume
float_t VelocityToVolume(uint8_t velocity)
{
    return (1.0f / (float)MIDI_POLYPHONY) * (1.0f / 127.0f) * (float_t)velocity; 
}

/* ----------------------------------------------------------------------------*/
/* Private functions ----------------------------------------------------------*/

/// @brief Turns note on.
void NoteOn(uint8_t noteIdx, uint8_t velocity)
{
    if (velocity == 0) return;

    //@TODO: Voice stealing.
    for (int i = 0; i < MIDI_POLYPHONY; i++)
    {
        if (gPlayingNotes[i].mNoteVelocity == 0)
        {
            gPlayingNotes[i].mNoteIdx = noteIdx;
            gPlayingNotes[i].mNoteVelocity = velocity;
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
            gPlayingNotes[i].mNoteIdx = 0;
            break;
        }
    }
}