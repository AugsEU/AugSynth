#ifndef __SVFILTER_H
#define __SVFILTER_H

#include <stdint.h>
#include <math.h>

typedef struct
{
    float_t mLp;             // Low-pass state
    float_t mBp;             // Band-pass state
    float_t mHp;             // High-pass state
} SVFilter_t;

void SvfInit(SVFilter_t* pFilter);
float_t SvfProcess(SVFilter_t* pFilter, float_t sample, float_t freq, float_t res, float_t mode);


#endif // __SVFILTER_H