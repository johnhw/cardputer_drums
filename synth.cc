#include <M5Cardputer.h>
#include <math.h>
#include "synth.h"


float iirAlpha(int sr, float freq)
{
  return 1.0 / (1.0 + (sr / (2 * M_PI * freq)));
}

float halfLifeTime(int sr, float t)
{
  return exp(-log(2.0) / ((float)t * (float)sr + 0.0f));
}

// times in milliseconds, frequencies in hZ
void createDrum(sample_t *sample, int32_t len, int32_t samplerate, float delay, float startFreq, float ampAttack, float freqDecay, float ampDecay, float noise, float overdrive)
{
  int32_t j;
  float freq = startFreq;
  float amp = 1.0;
  float ph = 0.0;
  float out;
  int32_t delaySamples;
  delaySamples = delay * samplerate / 1000;
  float ampD = halfLifeTime(samplerate, ampDecay / 1000);
  float freqD = halfLifeTime(samplerate, freqDecay / 1000);
  float ampAttackInc = 1.0 / (samplerate * (ampAttack / 1000));
  float t = 0;

  for (j = delay; j < sample->len; j++)
  {
    ph += (freq * 2 * M_PI) / samplerate;
    out = cos(ph);
    out += noise * random(-32767, 32767) / 32767.0;
    out *= amp;
    out *= overdrive;
    if (out > 1.0)
      out = 1.0;
    if (out < -1.0)
      out = -1.0;
    out += sample->samples[j] / 32767.0; // accumulate on top of existing sample
    sample->samples[j] = tanh(out) * 32767.0f;

    freq *= freqD;
    amp *= ampD;
    // if(t>ampAttack)
    //   amp *= ampD; // exp decay
    // else
    //  amp += ampAttackInc; // linear attack

    t += 1000 / samplerate;
  }
}

// times in milliseconds, frequencies in hZ
void createFM(sample_t *sample, int32_t len, int32_t samplerate, float freq, float ampAttack, float modRatio, float modStart, float modDecay, float ampDecay)
{
  int32_t j;

  float amp = 1.0;
  float ph = 0.0;
  float modPh = 0.0;
  float out;

  float mod = modStart;
  float modFreq = freq * modRatio;
  float ampD = halfLifeTime(samplerate, ampDecay / 1000);
  float modD = halfLifeTime(samplerate, modDecay / 1000);
  float ampAttackInc = 1.0 / (samplerate * (ampAttack / 1000));
  float t = 0;

  for (j = 0; j < sample->len; j++)
  {
    ph += (freq * 2 * M_PI) / samplerate;
    modPh += (modFreq * 2 * M_PI) / samplerate;
    out = cos(ph + cos(modPh) * mod);
    out *= amp;
    out += sample->samples[j] / 32767.0; // accumulate on top of existing sample
    sample->samples[j] = tanh(out) * 32767.0f;

    mod = mod * modD;
    // if(t>ampAttack)
    amp *= ampD; // exp decay
    // else
    // amp += ampAttackInc; // linear attack

    t += 1000 / samplerate;
  }
}