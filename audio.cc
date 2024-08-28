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
    int32_t len = findSampleEnd(dm.audioBuffers[0], dm.waveBufferLen, 100, 0.03);
    allocSample(dm, drumIndex, len);
    memcpy(dm.drumSamples[drumIndex].samples, dm.audioBuffers[0], len * sizeof(int16_t));
    memset(dm.audioBuffers[0], 0, dm.waveBufferLen * sizeof(int16_t));
    return;
}


void makeSynth(DrumMachine &dm, int index, synth_t *synth)
{
    sample_t scratchSample;
    scratchSample.samples = dm.audioBuffers[0];
    scratchSample.len = dm.waveBufferLen;
    createSynth(&scratchSample, samplerate, synth);
    autoSample(dm, index);
}



void createSamples(DrumMachine& dm, kit_t &kit)
{

   // initialise with dummy samples
  for(int i=0;i<26;i++)
  {
    allocSample(dm, i, 1);
  }

  // 8 semitones from middle-c
  int16_t noteFreqs[] = {261, 294, 330, 350, 392, 440, 493, 523};

  synth_t bass = kit.synths[0];

  // sample 0 is empty and not allocated

  // kit element 0 is the bass and repeated 8 times, for slots 1-9
  for(int i=1;i<9;i++)
  {
    bass.startFreq = noteFreqs[i-1] * 0.25;
    makeSynth(dm, i, &bass);
  }

  // the remaining 17 are the drum kit
  for(int i=9;i<26;i++)    
  {
    bass = kit.synths[i-8];
    makeSynth(dm, i, &bass);
  }

   
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