#include <Envelope.h>

extern float gParameters[128];

void EnvInit(Envelope_t* pEnv)
{
    pEnv->mSection = ES_OFF; 
    pEnv->mAttack = 0.01f;
    pEnv->mSustain = 0.95f;
    pEnv->mDecay = 0.01f;
    pEnv->mRelease = 0.01f;
    pEnv->mVolume = 0.0f;
}

void EnvNextSample(Envelope_t* pEnv)
{
    EnvelopeSection_t section = pEnv->mSection;

    switch (section)
    {
    case ES_OFF:
        break;
    case ES_ATTACK:
        pEnv->mVolume += pEnv->mAttack;
        if(pEnv->mVolume >= 1.0f)
        {
            pEnv->mVolume = 1.0f;
            pEnv->mSection = ES_DECAY;
        }
        break;
    case ES_DECAY:
        pEnv->mVolume -= pEnv->mDecay;
        if(pEnv->mVolume <= pEnv->mSustain)
        {
            pEnv->mVolume = pEnv->mSustain;
            uint32_t soundType = EXTRACT_INT_PARAM(gParameters, ASP_SOUND_TYPE);
            if(soundType == SOUND_TYPE_POLY || soundType == SOUND_TYPE_MONO)
            {
                pEnv->mSection = ES_SUSTAIN;
            }
            else
            {
                pEnv->mSection = ES_RELEASE;
            }
        }
        break;
    case ES_SUSTAIN:
        if(pEnv->mSustain == 0.0f)
        {
            pEnv->mSection = ES_OFF;
        }
        break; // Something else has to set this release.
    case ES_RELEASE:
        pEnv->mVolume -= pEnv->mRelease;
        if(signbit(pEnv->mVolume))
        {
            pEnv->mVolume = 0.0f;
            pEnv->mSection = ES_OFF;
        }
        break;
    }
}