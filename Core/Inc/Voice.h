#ifndef __VOICE_H
#define __VOICE_H

/* ----------------------------------------------------------------------------*/
/* Includes -------------------------------------------------------------------*/
#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include <Oscillator.h>
#include <Envelope.h>


/* ----------------------------------------------------------------------------*/
/* Types ----------------------------------------------------------------------*/
typedef struct
{
    float_t mFreq; // Note: Oscillator frequency may vary from this.
    uint8_t mPlayingNoteIdx;

    Oscillator_t mOsc1;
    Envelope_t mEnv1;

    Oscillator_t mOsc2;
    Envelope_t mEnv2;

    float_t mLfoDelta;
    float_t mLfoAmount;
} Voice_t;

void VoiceInit(Voice_t* pVoice);

void VoiceOn(Voice_t* pVoice, uint8_t playingNoteIdx);
void VoiceOff(Voice_t* pVoice);

void VoicePrepSampleBlock(Voice_t* pVoice);
float_t VoiceGetSample(Voice_t* pVoice, 
    uint32_t waveShape1, uint32_t waveShape2, 
    float_t tune1, float_t tune2, 
    float_t shape1, float_t shape2,
    float_t lfoValue);

int VoiceStealPriority(Voice_t* pVoice, uint8_t noteIdx);

#endif // __VOICE_H