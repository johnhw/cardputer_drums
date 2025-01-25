#ifndef __FX_H__
#define __FX_H__
#include <math.h>
#include <stdint.h>

typedef struct compressor_t
{
    float threshold_dB;
    float ratio;
    float makeup_dB;
    float attack_ms;
    float release_ms;
    float sampleRate;
    float rms;
} compressor_t;

void dynamicCompressFX(int16_t *samples, int32_t numSamples, compressor_t *compressor);

#endif // __FX_H__