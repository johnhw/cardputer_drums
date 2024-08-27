#include <M5Cardputer.h>
#include "utils.h"
#include "datatypes.h"
#include "config.h"
#include "synth.h"
#include "pattern.h"



// threshold is 0.0 to 1.0. all other values in samples
int32_t findSampleEnd(int16_t *sample, int32_t len, int32_t win, double threshold)
{    
    // work backward through the array, stepping by win each time
    for (int32_t i = len - win; i > 0; i -= win)
    {
        // if the mean of the absolute values of the samples in the window is less than the threshold, we're done
        double sum = 0;
        for (int32_t j = 0; j < win; j++)
        {
            sum += abs(sample[i + j]) / 32767.0;
        }
        sum /= win;
        if (sum > threshold)
        {
            return i + win;
        }
    }
    return 0;
}



void allocSample(DrumMachine& dm, int8_t index, int32_t len) {
    dm.drumSamples[index].samples = (int16_t*)allocBuffer(dm.drumSamples[index].samples, len, sizeof(int16_t));
    dm.drumSamples[index].len = len;
    dm.drumSamples[index].freqIncrement = 32768; // for fixed point frequency shifting
}

void allocateMix(DrumMachine& dm) {
    int32_t bufSamples;
    int32_t maxPatSamples;
    maxPatSamples = nSteps * samplerate * 60 / (minBPM * 4);
    bufSamples = maxPatSamples / 4;
    for (int i = 0; i < 4; i++) {
        dm.audioBuffers[i] = (int16_t*)allocBuffer(dm.audioBuffers[i], bufSamples, sizeof(int16_t));
    }
    dm.waveBufferIndex = 0;
}

// find the true length of the sample, and allocate it
// then copy in the data from the scratch buffer
void autoSample(DrumMachine &dm, int drumIndex)
{
    // use audiobuffers as a scratch space
    int32_t len = findSampleEnd(dm.audioBuffers[0], dm.waveBufferLen, 100, 0.05);
    allocSample(dm, drumIndex, len);
    memcpy(dm.drumSamples[drumIndex].samples, dm.audioBuffers[0], len * sizeof(int16_t));
    return;
}

void makeDrum(DrumMachine &dm, int index, float delay, float startFreq, float ampAttack, float freqDecay, float ampDecay, float noise, float overdrive)
{
    // synthesise into the scratch buffer then allocate just enough to encompass it
    sample_t scratchSample;
    scratchSample.samples = dm.audioBuffers[0];
    scratchSample.len = dm.waveBufferLen;
    createDrum(&scratchSample, samplerate, delay, startFreq, ampAttack, freqDecay, ampDecay, noise, overdrive);
    autoSample(dm, index);
    memset(dm.audioBuffers[0], 0, dm.waveBufferLen * sizeof(int16_t));
}

void makeFM(DrumMachine &dm, int index, float freq, float ampAttack, float modRatio, float modStart, float modDecay, float ampDecay)
{
    sample_t scratchSample;
    scratchSample.samples = dm.audioBuffers[0];
    scratchSample.len = dm.waveBufferLen;
    // synthesise into the scratch buffer then allocate just enough to encompass it
    createFM(&scratchSample, samplerate, freq, ampAttack, modRatio, modStart, modDecay, ampDecay);
    autoSample(dm, index);
    memset(dm.audioBuffers[0], 0, dm.waveBufferLen * sizeof(int16_t));
}

void createSamples(DrumMachine& dm)
{
  for (int i = 0; i < 19; i++)
  {
    allocSample(dm, i, 2200); // 0=silence
  }

  // 8 semitones from middle-c
  int16_t noteFreqs[] = {261, 294, 330, 350, 392, 440, 493, 523};

  for (int i = 1; i < 9; i++)
  {
    makeFM(dm, i, noteFreqs[i - 1] * 0.25, 30, 2.0, 8.0, 50, 80);    
  }
  makeDrum(dm, 9, 0.0, 450, 0.1, 15, 50, 0.0, 2.5);    // kick i
  makeDrum(dm, 10,  0.0, 0, 0.1, 0, 8, 1.0, 0.25);       // hihat j
  makeDrum(dm, 11,  0.0, 300, 0.1, 100, 18, 1.0, 3.6);  // snare k
  makeDrum(dm, 12,  0.0, 900, 0.05, 1000, 3, 0.0, 1.0); // click l
  makeDrum(dm, 13,  0.0, 400, 2, 1000, 15, 0.0, 1.0);   // tom m
  makeDrum(dm, 14,  0.0, 2000, 1, 200, 200, 0.2, 0.25); // sweep
  makeDrum(dm, 15,  0.0, 1000, 0.1, 200, 3, 0.0, 1.7);  // teek
  makeDrum(dm, 16,  0.0, 0.0, 20, 0.0, 300, 1.0, 0.1); // open hat

  // clap
  makeDrum(dm, 17,  60, 0, 0.1, 100, 19, 1.0, 2.4); // clap
  createDrum(&dm.drumSamples[17],  samplerate, 0.0, 0, 0.1, 100, 18, 1.0, 8.6);
  createDrum(&dm.drumSamples[17],  samplerate, 25, 0, 0.1, 100, 19, 1.0, 8.9);
  createDrum(&dm.drumSamples[17],  samplerate, 55, 0, 0.1, 100, 17, 1.0, 8.4);
  
  makeDrum(dm, 18, 0.0, 180, 0.1, 200, 60, 0.0, 3.5); // low kick

}


void mix(DrumMachine& dm)
{
    int chan, i, j, k;
    mixData_t mixData[nChans];
    sample_t *newSample;
    int32_t out;
    int32_t gainDrop = 64;
    int32_t bufferIndex;
    int16_t *buffer;
    int16_t newIndex;

    // reset the mix data
    for (chan = 0; chan < nChans; chan++)
    {
        mixData[chan].currentSample = nullptr;
        mixData[chan].sampleIndex = 0;
        mixData[chan].stepIndex = -1;
        mixData[chan].nextIndex = -1;
        mixData[chan].kickDelay = 0;
        mixData[chan].currentVelocity = 0;
    }
    k = 0;
    // for each buffer
    for (i = 0; i < 4; i++)
    {
        buffer = dm.audioBuffers[i];

        for (j = 0; j < dm.waveBufferLen; j++)
        {
            out = 0;
            for (chan = 0; chan < nChans; chan++)
            {
                if (!dm.channels[chan]._enabled)
                    continue; // skip disabled channels
                newIndex = k / dm.stepSamples;
                if (newIndex != mixData[chan].nextIndex)
                {
                    mixData[chan].nextIndex = newIndex;
                    if (newIndex % 2 == 0)
                        mixData[chan].kickDelay = 0; // no swing on downbeats
                    else
                        mixData[chan].kickDelay = dm.stepSamples * dm.swing / 200; // apply swing (max=50% delay)
                    mixData[chan].kickDelay += getKickDelay(dm, newIndex, chan) * dm.oneKickTime; // add on forced kick delay in the channel
                }

                // did it change yet? if so, we need a new sample
                if (mixData[chan].kickDelay == 0)
                {
                    // returns NULL if there's no sample there at all
                    mixData[chan].stepIndex = mixData[chan].nextIndex;
                    getStep(dm, mixData[chan].stepIndex, chan, newSample, mixData[chan].currentVelocity);
                    if (newSample != nullptr) // cutoff if there's a new sample to start (do nothing otherwise)
                    {
                        mixData[chan].sampleIndex = 0;
                        mixData[chan].currentSample = newSample;
                    }
                }
                mixData[chan].kickDelay--;

                // copy in the sample, if there's more to copy
                if (mixData[chan].currentSample)
                {
                    out += mixData[chan].currentVelocity * mixData[chan].currentSample->samples[mixData[chan].sampleIndex++];
                    // overran the sample, so stop
                    if (mixData[chan].sampleIndex >= mixData[chan].currentSample->len)
                    {
                        mixData[chan].currentSample = nullptr;
                    }
                }
            }
            // shift to a 16 bit result, clip and write
            out = out / gainDrop;
            if (out > 32767)
                out = 32767;
            if (out < -32767)
                out = -32767;
            buffer[j] = out;
            k++;
        }
    }
}


// feed the audio buffers with the mixed audio
void feedPatternBuffers(DrumMachine &dm)
{
  // room in the queue? stuff the buffers
  while (M5Cardputer.Speaker.isPlaying(0) != 2)
  {
    M5Cardputer.Speaker.playRaw(dm.audioBuffers[dm.waveBufferIndex++], dm.waveBufferLen, samplerate, false, 1, 0);
    if (dm.waveBufferIndex == 2)
    {
      if (dm.syncMix) // mix in time for the next buffer change
      {
        dm.syncMix = 0;
        mix(dm);
      }
  
      dm.syncMillis = millis();
    }
    if (dm.waveBufferIndex > 3)
    {
      dm.waveBufferIndex = 0;
    }
    M5Cardputer.update();
  }
}