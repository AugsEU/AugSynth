#ifndef __ENVELOPE_H
#define __ENVELOPE_H

/* ----------------------------------------------------------------------------*/
/* Includes -------------------------------------------------------------------*/
#include <stdint.h>
#include <stdbool.h>
#include <math.h>



/* ----------------------------------------------------------------------------*/
/* Types ----------------------------------------------------------------------*/
typedef enum EnvelopeSection
{
    ES_OFF,
    ES_RELEASE,
    ES_ATTACK,
    ES_DECAY,
    ES_SUSTAIN
} EnvelopeSection_t;

typedef struct
{
    EnvelopeSection_t mSection;
    float_t mAttack;
    float_t mDecay;
    float_t mSustain;
    float_t mRelease;

    float_t mVolume;
} Envelope_t;

void EnvInit(Envelope_t* pEnv);
void EnvNextSample(Envelope_t* pEnv);

#endif // __ENVELOPE_H