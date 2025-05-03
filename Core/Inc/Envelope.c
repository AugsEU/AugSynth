#include <Envelope.h>

void EnvInit(Envelope_t* pEnv)
{
    pEnv->mSection = ES_OFF; 
    pEnv->mAttack = 0.1f;
    pEnv->mSustain = 0.1f;
    pEnv->mDecay = 0.1f;
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
            pEnv->mSection = ES_SUSTAIN;
        }
        break;
    case ES_SUSTAIN:
        break; // Something else has to set this release.
    case ES_RELEASE:
        pEnv->mVolume -= pEnv->mDecay;
        if(pEnv->mVolume <= 0.0f)
        {
            pEnv->mVolume = 0.0f;
            pEnv->mSection = ES_OFF;
        }
        break;
    }
}