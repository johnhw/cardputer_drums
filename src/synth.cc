#include <M5Cardputer.h>
#include <math.h>
#include "synth.h"



inline float randFloat()
{
  return random(-32767, 32767) / 32767.0;
}


void createSynth(sample_t *sample, int32_t samplerate, synth_t *synth)
{

  int32_t i;
  int32_t sampleOffset=0, sampleStep;
  int32_t maxRandomDelaySamples = synth->retriggerRandomDelay * samplerate / 1000;
  float ampScale = 1.0;   
    // if we have multiple retriggers, we need to calculate the sample offset and amplitude scale for each
  sampleStep = synth->retriggersDelay * samplerate / 1000;
  if(synth->retriggers<2)
    subCreateSynth(sample, samplerate, synth, 0, 1.0);
  else
  {    
    // otherwise, overlay multiple retriggered version
    for(i=0; i<synth->retriggers; i++)
    {
      subCreateSynth(sample, samplerate, synth, sampleOffset, ampScale);
      sampleOffset += sampleStep;
      sampleOffset += random(-maxRandomDelaySamples, maxRandomDelaySamples);
      if(sampleOffset<0) sampleOffset = 0;
      ampScale *= synth->retriggerDecay;
    }
  }
}

void subCreateSynth(sample_t *sample, int32_t samplerate, synth_t *synth, int32_t sampleOffset, float ampScale)
{
  int32_t j;
  
  float amp = 0.0;
  float ph = 0.0;
  float modPh = 0.0;
  float filterState = 0.0;
  float out;
  int32_t delaySamples;
  float freq, modFreq;
  delaySamples = synth->delay * samplerate / 1000;
  float ampAttack = synth->ampAttack;
  float freqDecay = synth->freqDecay;
  float startFreq = synth->startFreq;
  float endFreq = synth->endFreq;
  float freqOffset = startFreq - endFreq;
  float ampDecay = synth->ampDecay;
  float modStart = synth->modStart;
  float modDecay = synth->modDecay;
  float modEnd = synth->modEnd;
  float modRatio = synth->modRatio;
  float noise = synth->noise;
  float overdrive = synth->overdrive;
  float lowpassStart = synth->lowpassStart;
  float lowpassEnd = synth->lowpassEnd;
  float lowpassOffset = synth->lowpassStart - synth->lowpassEnd;
  float lowpassDecay = synth->lowpassDecay;

  
  float modOffset = modEnd - modStart;
  float mod;
  if(ampAttack==0.0) ampAttack = 0.1;
  if(freqDecay==0.0) freqDecay = 10000000;
  if(modDecay==0.0) modDecay = 100000000;
  if(lowpassDecay==0.0) lowpassDecay = 100000000;
  

  float ampD = halfLifeTime(samplerate, ampDecay / 1000);
  float freqD = halfLifeTime(samplerate, freqDecay / 1000);
  float modD = halfLifeTime(samplerate, modDecay / 1000);
  float lowpassD = halfLifeTime(samplerate, lowpassDecay / 1000);
  float ampAttackInc = 1.0 / (samplerate * (ampAttack / 1000));  
  int attack = 1;

  float lowpassAlpha = 0.0;
  float lowpass;
  for (j = delaySamples + sampleOffset; j < sample->len; j++)  
  {
    lowpass = lowpassEnd + lowpassOffset;
    if(lowpass>0)
      lowpassAlpha = iirAlpha(samplerate, lowpass);
    else
      lowpassAlpha = 0.0;
    freq = endFreq + freqOffset;
    mod = modEnd + modOffset;
    ph += (freq * 2 * M_PI) / samplerate;
    modFreq = freq * modRatio;
    modPh += (modFreq * 2 * M_PI) / samplerate;

    out = cos(ph + cos(modPh) * mod);

    out += noise * randFloat();
    out *= amp;
    out *= overdrive;
    if (out > 1.0)
      out = 1.0;
    if (out < -1.0)
      out = -1.0;
    out = out * ampScale; // used for retriggers
    out += sample->samples[j] / 32767.0; // accumulate on top of existing sample
    out = tanh(out);
    filterState = lowpassAlpha * filterState + (1-lowpassAlpha) * out;
    sample->samples[j] = filterState * 32767.0f;

    freqOffset *= freqD;
    modOffset *= modD;
    lowpassOffset *= lowpassD;
    if(amp < 1.0 && attack)
    {
      amp += ampAttackInc;
    }
    else
    {
      attack = 0;
      amp *= ampD;
    }
  }

}
