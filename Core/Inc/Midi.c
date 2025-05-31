#include "Midi.h"
#include "Constants.h"
#include "Voice.h"
#include "Tuning.h"
#include <string.h>

/* ----------------------------------------------------------------------------*/
/* Global variables -----------------------------------------------------------*/
#define NOTE_STACK_SIZE 64
extern Voice_t gVoiceBank[MIDI_POLYPHONY];
extern bool gClickEnabled;
extern uint8_t gPendingVoiceSteal;
float gParameters[128];
uint8_t gNoteStack[NOTE_STACK_SIZE];
uint8_t gNoteStackIdx = 0;

void NoteOn(uint8_t noteIdx, uint8_t velocity);
void NoteOff(uint8_t noteIdx);

/* ----------------------------------------------------------------------------*/
/* Public functions -----------------------------------------------------------*/

/// @brief Initialise midi
void MidiInit(void)
{

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
    case MIDI_CLICK_ON:
        gClickEnabled = true;
        break;
    case MIDI_CLICK_OFF:
        gClickEnabled = false;
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

    uint32_t soundType = EXTRACT_INT_PARAM(gParameters, ASP_SOUND_TYPE);
    if(soundType == SOUND_TYPE_MONO || soundType == SOUND_TYPE_BASS) // Mono sound type
    {
        gNoteStack[gNoteStackIdx] = noteIdx;
        VoiceOn(&gVoiceBank[0], noteIdx);
        if(gNoteStackIdx < NOTE_STACK_SIZE - 1)
        {
            gNoteStackIdx++;
        }
    }
    else // Poly sound type
    {
        int bestVoiceIdx = 0;
        float_t bestVoicePrio = 0;
        for (int i = 0; i < MIDI_POLYPHONY; i++)
        {
            Voice_t* pVoice = &gVoiceBank[i];
            float_t currPrio = VoiceStealPriority(pVoice, noteIdx);

            if (currPrio > bestVoicePrio)
            {
                bestVoiceIdx = i;
                bestVoicePrio = currPrio;
                if(currPrio >= 8.0f) break;
            }
        }

        if (bestVoicePrio > 0)
        {
            VoiceOn(&gVoiceBank[bestVoiceIdx], noteIdx);
        }
    }
}

/// @brief Turns note off.
void NoteOff(uint8_t noteIdx)
{
    uint32_t soundType = EXTRACT_INT_PARAM(gParameters, ASP_SOUND_TYPE);
    if(soundType == SOUND_TYPE_MONO || soundType == SOUND_TYPE_BASS) // Mono sound type
    {
        if(gNoteStackIdx <= 1)
        {
            VoiceOff(&gVoiceBank[0]);
            gNoteStackIdx = 0;
            return;
        }

        gNoteStackIdx--;
        for (int i = 0; i < gNoteStackIdx; i++)
        {
            if(gNoteStack[i] == noteIdx)
            {
                memcpy(gNoteStack + i, gNoteStack + i + 1, (gNoteStackIdx-i)*sizeof(uint8_t));
                return;
            }
        }

        if(gNoteStack[gNoteStackIdx] == noteIdx)
        {
            VoiceOnSteal(&gVoiceBank[0], gNoteStack[gNoteStackIdx-1]);
        }
    }
    else // Poly sound type
    {
        for (int i = 0; i < MIDI_POLYPHONY; i++)
        {
            if (gVoiceBank[i].mPlayingNoteIdx == noteIdx)
            {
                VoiceOff(&gVoiceBank[i]);
            }
        }
    }
}