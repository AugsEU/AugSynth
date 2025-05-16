#include <Voice.h>
#include <Midi.h>
#include <Params.h>
#include <Tuning.h>

extern float gParameters[128];

/// @brief Initialise the voice at program start.
void VoiceInit(Voice_t* pVoice)
{
    pVoice->mPlayingNoteIdx = 0;
    pVoice->mFreq = 0.0f;

    OscInit(&pVoice->mOsc1);
    EnvInit(&pVoice->mEnv1);

    OscInit(&pVoice->mOsc2);
    EnvInit(&pVoice->mEnv2);
}

/// @brief Begin playing voice.
void VoiceOn(Voice_t* pVoice, uint8_t playingNoteIdx)
{
    pVoice->mEnv1.mSection = ES_ATTACK;
    pVoice->mEnv2.mSection = ES_ATTACK;

    bool sameNote = playingNoteIdx == pVoice->mPlayingNoteIdx;
    if (!sameNote)
    {
        pVoice->mEnv2.mVolume = 0.0f;
        pVoice->mEnv1.mVolume = 0.0f;
    }

    pVoice->mLfoAmount = 0.0f;

    pVoice->mPlayingNoteIdx = playingNoteIdx;
    pVoice->mFreq = NoteToFreq12TET(pVoice->mPlayingNoteIdx) * SAMPLE_PERIOD;
}

void VoiceOff(Voice_t* pVoice)
{
    pVoice->mEnv1.mSection = ES_RELEASE;
    pVoice->mEnv2.mSection = ES_RELEASE;
}

void VoicePrepSampleBlock(Voice_t* pVoice)
{
    pVoice->mEnv1.mAttack = gParameters[ASP_ENV_ATTACK1];
    pVoice->mEnv1.mSustain = gParameters[ASP_ENV_SUSTAIN1];
    pVoice->mEnv1.mDecay = gParameters[ASP_ENV_DECAY1];

    pVoice->mEnv2.mAttack = gParameters[ASP_ENV_ATTACK2];
    pVoice->mEnv2.mSustain = gParameters[ASP_ENV_SUSTAIN2];
    pVoice->mEnv2.mDecay =  gParameters[ASP_ENV_DECAY2];

    pVoice->mLfoDelta = gParameters[ASP_LFO_ATTACK];
}

float_t VoiceGetSample(Voice_t* pVoice, float_t waveShape1, float_t waveShape2, float_t tune1, float_t tune2, float_t lfoValue, float_t lfoGain)
{
    if(pVoice->mLfoAmount < 1.0f)
    {
        pVoice->mLfoAmount += pVoice->mLfoDelta;
    }
    lfoValue *= pVoice->mLfoAmount;

    float_t osc1TuneLFO = FastUnitExp(gParameters[ASP_LFO_OSC1_TUNE] * lfoValue); //@TODO precompute for all oscillators
    float_t osc2TuneLFO = FastUnitExp(gParameters[ASP_LFO_OSC2_TUNE] * lfoValue);
    float_t dt =  pVoice->mFreq;

    // Osc1
    OscPhaseInc(&pVoice->mOsc1, dt * tune1 * osc1TuneLFO);
    float_t osc1st;
    if (waveShape1 < 0.0f)
    {
        osc1st = OscSquare(&pVoice->mOsc1, dt);
        waveShape1 = -waveShape1;
    }
    else
    {
        osc1st = OscSawTooth(&pVoice->mOsc1, dt);
    }
    float_t osc1si = OscSine(&pVoice->mOsc1);

    osc1st *= waveShape1;
    osc1si *= (1.0f - waveShape1);

    osc1si += osc1st;
    osc1si *= gParameters[ASP_DCO_VOL_1];

    EnvNextSample(&pVoice->mEnv1);
    osc1si *= pVoice->mEnv1.mVolume;

    // Osc2
    OscPhaseInc(&pVoice->mOsc2, dt * tune2 * osc2TuneLFO);
    float_t osc2st = OscSquare(&pVoice->mOsc2, dt);
    float_t osc2si = OscSine(&pVoice->mOsc2);

    osc2st *= waveShape2;
    osc2si *= (1.0f - waveShape2);

    osc2si += osc2st;
    osc2si *= gParameters[ASP_DCO_VOL_2];
    
    EnvNextSample(&pVoice->mEnv2);
    osc2si *= pVoice->mEnv2.mVolume;

    return (osc2si + osc1si) * (1.0f - lfoGain * (lfoValue + 1.0f));
}

int VoiceStealPriority(Voice_t* pVoice, uint8_t noteIdx)
{
    int priority = 2 - (int)pVoice->mEnv1.mSection;
    priority += 2 - (int)pVoice->mEnv2.mSection;

    if (pVoice->mPlayingNoteIdx == noteIdx)
    {
        priority += 5;
    }

    return priority;
}