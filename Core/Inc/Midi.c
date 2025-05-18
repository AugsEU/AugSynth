#include "Midi.h"
#include "Constants.h"
#include "Voice.h"
#include "Tuning.h"

/* ----------------------------------------------------------------------------*/
/* Global variables -----------------------------------------------------------*/
extern Voice_t gVoiceBank[MIDI_POLYPHONY];
float gParameters[128];

void NoteOn(uint8_t noteIdx, uint8_t velocity);
void NoteOff(uint8_t noteIdx);

/* ----------------------------------------------------------------------------*/
/* Public functions -----------------------------------------------------------*/

/// @brief Initialise midi
void MidiInit(void)
{
    // for (int i = 0; i < MIDI_POLYPHONY; i++)
    // {
    //     gPlayingNotes[i].mNoteIdx = 0xFF;
    //     gPlayingNotes[i].mNoteVelocity = 0;
    // }
}



/// @brief Process a midi message.
void ProcessMidiMessage(uint8_t* buff)
{
    uint8_t header = buff[0];
    float paramValue;
    uint32_t buff32;
    uint32_t* param32;

    switch (header)
    {
    case MIDI_NOTE_ON:
        NoteOn(buff[1], buff[2]);
        break;
    case MIDI_NOTE_OFF:
        NoteOff(buff[1]);
        break;
    // case MIDI_INT_PARAM:
    //     buff32 = buff[1];
    //     param32 = (uint32_t*)(&gParameters[header & MIDI_PARAM_NUM_MASK]);
    //     *param32 = buff32;
    //     break;
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

    int bestVoiceIdx = 0;
    int bestVoicePrio = 0;
    for (int i = 0; i < MIDI_POLYPHONY; i++)
    {
        Voice_t* pVoice = &gVoiceBank[i];
        int currPrio = VoiceStealPriority(pVoice, noteIdx);

        if (currPrio > bestVoicePrio)
        {
            bestVoiceIdx = i;
            bestVoicePrio = currPrio;
        }
    }

    if (bestVoicePrio > 0)
    {
        VoiceOn(&gVoiceBank[bestVoiceIdx], noteIdx);
    }
}

/// @brief Turns note off.
void NoteOff(uint8_t noteIdx)
{
    for (int i = 0; i < MIDI_POLYPHONY; i++)
    {
        if (gVoiceBank[i].mPlayingNoteIdx == noteIdx)
        {
            VoiceOff(&gVoiceBank[i]);
        }
    }
}