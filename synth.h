#ifndef SYNTH_H
#include <stdint.h>
#include "datatypes.h"

float iirAlpha(int sr, float freq);
float halfLifeTime(int sr, float t);

// times in milliseconds, frequencies in hZ
void createDrum(sample_t *sample, int32_t samplerate, float delay, float startFreq, float ampAttack, float freqDecay, float ampDecay, float noise, float overdrive);
void createFM(sample_t *sample,  int32_t samplerate, float freq, float ampAttack, float modRatio, float modStart, float modDecay, float ampDecay);
#endif