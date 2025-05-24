#include "NLFilter.h"
#include "Params.h"

#define NLFILTER_FREQ_MAX 0.73f
#define FAST_FILTER_CLIP 1

float_t FilterClip(float_t n);
NLFilter_t gNLFilter;

void InitFilter()
{
	gNLFilter.mType = FILTER_MODE_OFF;
	gNLFilter.mSample0 = 0;
	gNLFilter.mSample1 = 0;
	gNLFilter.mFreq = 0.2f;
	gNLFilter.mQ = 0.9f;
}

void SetFilterRes(float_t val)
{
	gNLFilter.mQ = 1.0f - val;
	if(gNLFilter.mQ < 0.01f)
    {
        gNLFilter.mQ = 0.01f;
    }
}

void SetFilterFreq(float_t val)
{
	val *= (NLFILTER_FREQ_MAX * 0.5f);
	gNLFilter.mFreq = val;
	float_t g = val * val; // Gain = 18xxx + 3x
    g *= val;
    g *= 18.0f;
    val *= 3.0f;

	gNLFilter.mGain = g + val;
}

void SetFilterType(uint8_t val)
{
	gNLFilter.mType = val;
}

float_t CalcFilterSample(float_t smpl)
{
    if(gNLFilter.mType == FILTER_MODE_OFF) // Low pass
	{
        return smpl;
    }

	float_t out;
	float_t r;
	const float_t fg 	= gNLFilter.mGain;
    const float_t samp0 = gNLFilter.mSample0;
	const float_t samp1 = gNLFilter.mSample1;

	if (gNLFilter.mFreq >= 0.4499f)
    {
        r = 1.0f;
    }
	else
    {
        r= gNLFilter.mQ;
    } 

	const float_t df = fg*fg;
	const float_t s = FilterClip(smpl * 0.8f);
	const float_t d0 = 1.0f / (1.0f + 2.0f * fg * r);

	const float_t fdb = df * d0;
	float_t y1= (fdb * s+ samp1 + fg * d0 * samp0) / (fdb + 1.0f);
	const float_t dx = s - y1;
	const float_t y0 = (FilterClip(samp0) + fg * dx) * d0;

	gNLFilter.mSample0 = FilterClip(gNLFilter.mSample0) + 2.0f * fg * (dx - 2.0f * r * y0);
	gNLFilter.mSample1 += 2.0f * fg * y0;

	if(gNLFilter.mType == FILTER_MODE_LP) // Low pass
	{
		if(y1 < -1.95f) out= -1.0f;
		else if(y1 > 1.95f) out= 1.0f;
		else out=  4.15f*y1/(4.29f+y1*y1);
	}
	else // High pass
	{
		const float_t efg = 2*r*y0;
		const float_t h = s - efg - y1;
		out = h * FLTR_GAIN;

        if (out > 1.0f) out=1.0f;
	    if (out < -1.0f) out= -1.0f;
	}

	return out;
}

#if FAST_FILTER_CLIP
float_t FilterClip(float_t n)
{
    if(n < -4.0f) return -2.0f;
    else if(n > 4.0f) return 2.0f;
	
    float_t a=n*n;
    a *= 0.125f;

    if(signbit(n))
    {
        return n + a;
    }
    
    return n - a;
}
#else
float_t FilterClip(float_t n)
{
	float_t a=0.5*n;
	float_t b = a*a;
	a = ((b + 105)*b + 945) / ((15*b + 420)*b + 945);
	return n * a;
}
#endif
//------------------------------------------------------------------------------------
