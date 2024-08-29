#ifndef SYNTH_H
#define SYNTH_H
#include <stdint.h>
#include "datatypes.h"

float iirAlpha(int sr, float freq);
float halfLifeTime(int sr, float t);

typedef struct synth_t
{
  float delay;
  float startFreq;
  float endFreq;
  float freqDecay;
  float ampAttack;
  float ampDecay;
  float noise;  
  float modRatio;
  float modStart;
  float modEnd;
  float modDecay;  
  float overdrive;
  float lowpassStart;
  float lowpassEnd;
  float lowpassDecay;
} synth_t;

struct kit_t
{
  const synth_t *synths;
  const int nSynths;
  const char *name;
  const int32_t scaleFreqs[8][5]; // up to 5 note chords
};


// times in milliseconds, frequencies in hZ

void createSynth(sample_t *sample, int32_t samplerate, synth_t *synth);
inline float randFloat();


#endif