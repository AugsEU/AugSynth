#include <Voice.h>
#include <Midi.h>
#include <Params.h>
#include <Tuning.h>

#define MONO_OSC 0

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
    pVoice->mFreq = NoteToFreq(pVoice->mPlayingNoteIdx) * SAMPLE_PERIOD;
}

void VoiceOff(Voice_t* pVoice)
{
    pVoice->mEnv1.mSection = ES_RELEASE;
    pVoice->mEnv2.mSection = ES_RELEASE;
}

void VoicePrepSampleBlock(Voice_t* pVoice)
{
    pVoice->mEnv1.mAttack = gParameters[ASP_ENV_ATTACK1];
    pVoice->mEnv1.mDecay = gParameters[ASP_ENV_DECAY1];
    pVoice->mEnv1.mSustain = gParameters[ASP_ENV_SUSTAIN1];
    pVoice->mEnv1.mRelease = gParameters[ASP_ENV_RELEASE1];
    
    pVoice->mEnv2.mAttack = gParameters[ASP_ENV_ATTACK2];
    pVoice->mEnv2.mDecay =  gParameters[ASP_ENV_DECAY2];
    pVoice->mEnv2.mSustain = gParameters[ASP_ENV_SUSTAIN2];
    pVoice->mEnv2.mRelease = gParameters[ASP_ENV_RELEASE2];

    pVoice->mLfoDelta = gParameters[ASP_LFO_ATTACK];
}

float_t VoiceGetSample(Voice_t* pVoice, 
                        uint32_t waveType1, uint32_t waveType2, 
                        float_t tune1, float_t tune2, 
                        float_t shape1, float_t shape2,
                        float_t lfoValue, float_t lfoGain)
{
    if(pVoice->mLfoAmount < 1.0f)
    {
        pVoice->mLfoAmount += pVoice->mLfoDelta;
    }
    lfoValue *= pVoice->mLfoAmount;

    float_t osc1TuneLFO = FastUnitExp(gParameters[ASP_LFO_OSC1_TUNE] * lfoValue);
    float_t osc2TuneLFO = FastUnitExp(gParameters[ASP_LFO_OSC2_TUNE] * lfoValue);
    float_t dt =  pVoice->mFreq;

    // Osc1
    OscPhaseInc(&pVoice->mOsc1, dt * tune1 * osc1TuneLFO);
    float_t osc1;
    switch (waveType1)
    {
    default:
    case OSC_MODE_SINE:
        osc1 = OscSine(&pVoice->mOsc1);
        osc1 = ShapeWave(osc1, shape1);
        break;
    case OSC_MODE_SQUARE:
        osc1 = OscSquareBLEP(&pVoice->mOsc1, dt, shape1);
        break;
    case OSC_MODE_SAW:
        osc1 = OscSawTooth(&pVoice->mOsc1, dt);
        osc1 = ShapeWave(osc1, shape1);
        break;
    case OSC_MODE_ORGAN:
        osc1 = OscOrgan(&pVoice->mOsc1, shape1);
        break;
    }
    osc1 *= gParameters[ASP_DCO_VOL_1];

    EnvNextSample(&pVoice->mEnv1);
    osc1TuneLFO = pVoice->mEnv1.mVolume;// reuse var
    osc1 *= osc1TuneLFO * osc1TuneLFO;
    osc1 *= ComputeLfoMult(lfoValue, gParameters[ASP_LFO_OSC1_VOLUME]);

#if !MONO_OSC
    // Osc2
    OscPhaseInc(&pVoice->mOsc2, dt * tune2 * osc2TuneLFO);
    float_t osc2;
    switch (waveType2)
    {
    default:
    case OSC_MODE_SINE:
        osc2 = OscSine(&pVoice->mOsc2);
        osc2 = ShapeWave(osc2, shape2);
        break;
    case OSC_MODE_SQUARE:
        osc2 = OscSquareBLEP(&pVoice->mOsc2, dt, shape2);
        break;
    case OSC_MODE_SAW:
        osc2 = OscSawTooth(&pVoice->mOsc2, dt);
        osc2 = ShapeWave(osc2, shape2);
        break;
    case OSC_MODE_ORGAN:
        osc2= OscOrgan(&pVoice->mOsc2, shape1);
        break;
    }
    osc2 *= gParameters[ASP_DCO_VOL_2];

    EnvNextSample(&pVoice->mEnv2);
    osc2TuneLFO = pVoice->mEnv2.mVolume;// reuse var
    osc2 *= osc2TuneLFO * osc2TuneLFO;
    osc2 *= ComputeLfoMult(lfoValue, gParameters[ASP_LFO_OSC2_VOLUME]);

    return osc1 + osc2;
#else
    return osc1;
#endif
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