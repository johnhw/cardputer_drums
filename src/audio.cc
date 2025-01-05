#include <M5Cardputer.h>
#include "audio.h"
#include "utils.h"
#include "datatypes.h"
#include "config.h"
#include "synth.h"
#include "pattern.h"
#include "ui.h"
#include "flash.h"

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
    return win;
}

void allocSample(DrumMachine &dm, int8_t index, int32_t len)
{
    Serial.printf("Allocating sample %d, len %d\n", index, len);
    Serial.println("Free heap: " + String(ESP.getFreeHeap()));
    
    freeSample(dm, index);
    if(ESP.getFreeHeap() < len * sizeof(int16_t) + 4096)
    {
        Serial.println("Out of memory");
        dm.drumSamples[index].samples = nullptr;
        dm.drumSamples[index].len = 0;
        return;
    }
    dm.drumSamples[index].samples = (int16_t *)allocBuffer(dm.drumSamples[index].samples, len, sizeof(int16_t));
    if(dm.drumSamples[index].samples == nullptr)
    {
        Serial.println("Failed to allocate buffer");
        dm.drumSamples[index].len = 0;
        return;
    }
    dm.drumSamples[index].len = len;
    dm.drumSamples[index].detune = 0; 
}


void freeSample(DrumMachine &dm, int8_t index)
{
    if (dm.drumSamples[index].samples != nullptr)
    {
        free(dm.drumSamples[index].samples);
        dm.drumSamples[index].samples = nullptr;
        dm.drumSamples[index].len = 0;
    }
    dm.drumSamples[index].samples = nullptr;
    dm.drumSamples[index].len = 0;
}

void allocateMix(DrumMachine &dm)
{
    int32_t bufSamples;
    int32_t maxPatSamples;
    maxPatSamples = nSteps * samplerate * 60 / (minBPM * 4);
    bufSamples = maxPatSamples / 4;
    for (int i = 0; i < 4; i++)
    {
        dm.audioBuffers[i] = (int16_t *)allocBuffer(dm.audioBuffers[i], bufSamples, sizeof(int16_t));
    }
    dm.scratchBuffer = dm.audioBuffers[0];
    dm.bufferA = dm.audioBuffers[0];
    dm.bufferB = dm.audioBuffers[1];
    dm.waveBufferIndex = 0;
}

// find the true length of the sample, and allocate it
// then copy in the data from the scratch buffer
void autoSample(DrumMachine &dm, int drumIndex)
{
    int32_t maxSamples = samplerate * maxSampleLen;
    // use audiobuffers as a scratch space
    int32_t len = findSampleEnd(dm.audioBuffers[0], dm.waveBufferLen, 100, 0.03);
    len = min(len, maxSamples);        
    allocSample(dm, drumIndex, len);    
    memcpy(dm.drumSamples[drumIndex].samples, dm.audioBuffers[0], dm.drumSamples[drumIndex].len * sizeof(int16_t));    
    memset(dm.audioBuffers[0], 0, dm.waveBufferLen * sizeof(int16_t));    
    return;
}

void makeSynth(DrumMachine &dm, int index, synth_t *synth)
{
    sample_t scratchSample;
    scratchSample.samples = dm.audioBuffers[0];
    scratchSample.len = dm.waveBufferLen - 1;    
    createSynth(&scratchSample, samplerate, synth);    
    autoSample(dm, index);

}

void createSamples(DrumMachine &dm, kit_t &kit)
{

    // 8 semitones from middle-c
    int16_t noteFreqs[] = {261, 294, 330, 350, 392, 440, 493, 523};

    synth_t bass = kit.synths[0];

    // sample 0 is empty and not allocated

    // kit element 0 is the bass and repeated 8 times, for slots 1-9
    for (int i = 1; i < 9; i++)
    {        
        bass.startFreq = noteFreqs[i - 1] * 0.25;
        makeSynth(dm, i, &bass);
    }

    // the remaining 17 are the drum kit
    for (int i = 9; i < 26; i++)
    {
        bass = kit.synths[i - 8];        
        makeSynth(dm, i, &bass);
    }
    
}

float fast_tanh(float x)
{
    return 32767.0 * tanh(x / 32767.0);
}

// convert a detune in cents to a frequency increment
// where 32768.0 is a frequency of 1.0, 16384.0 is a frequency of 0.5, etc.
int32_t freqIncrement(int32_t totalDetune)
{
    float freq = powf(2.0, totalDetune / 1200.0);
    return freq * 32768.0;
}

// preview a sample by writing it into audioBuffer[0]
// using the freqIncrement etc. for detune
void previewSample(DrumMachine &dm, sample_t *preview)
{
    int32_t totalDetune = preview->detune;
    int32_t freqInc = freqIncrement(totalDetune);
    int32_t sampleIndex = 0;
    int32_t fractionalSampleIndex = 0;
    int32_t len = preview->len;
    int32_t out;
    float gain = 1.0;
    for (int i = 0; i < dm.waveBufferLen; i++)
    {
        if (sampleIndex >= len)
        {
            dm.audioBuffers[0][i] = 0;
            break;
        }
        float in = preview->samples[sampleIndex] * gain;
        fractionalSampleIndex += freqInc;
        sampleIndex = fractionalSampleIndex / 32768;
        out = in;
        if (out > 32767)
            out = 32767;
        if (out < -32767)
            out = -32767;
        dm.audioBuffers[0][i] = out;
    }
    M5Cardputer.Speaker.playRaw(dm.audioBuffers[0], len, samplerate, false, 1, 0);
}

void mix(DrumMachine &dm)
{
    int chan, i, j, k;
    mixData_t mixData[nChans];
    sample_t *newSample;
    int32_t out;
    int32_t gainDrop = 64;
    int32_t bufferIndex;
    int16_t *buffer;
    int16_t newIndex;
    int32_t totalDetune;

    // reset the mix data
    for (chan = 0; chan < nChans; chan++)
    {
        mixData[chan].currentSample = nullptr;
        mixData[chan].sampleIndex = 0;
        mixData[chan].stepIndex = -1;
        mixData[chan].nextIndex = -1;
        mixData[chan].kickDelay = 0;
        mixData[chan].gain = powf(1.4142, (dm.channels[chan].volume - 8));
        mixData[chan].currentVelocity = 0;
        mixData[chan].currentFilter = 0.0f;
        if (dm.channels[chan].filterCutoff == 0)
            mixData[chan].filterAlpha = 0.0f;
        else
            mixData[chan].filterAlpha = iirAlpha(samplerate, (maxFilterCutoff - dm.channels[chan].filterCutoff) * samplerate / (maxFilterCutoff * 8));
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
                        mixData[chan].kickDelay = dm.stepSamples * dm.swing / 200;                // apply swing (max=50% delay)
                    mixData[chan].kickDelay += getKickDelay(dm, newIndex, chan) * dm.oneKickTime; // add on forced kick delay in the channel
                }

                // did it change yet? if so, we need a new sample
                if (mixData[chan].kickDelay == 0)
                {
                    // returns NULL if there's no sample there at all
                    mixData[chan].stepIndex = mixData[chan].nextIndex;
                    getStep(dm, mixData[chan].stepIndex, chan, newSample, mixData[chan].currentVelocity);
                    getDetune(dm, mixData[chan].stepIndex, chan, mixData[chan].totalDetune);
                    if (newSample != nullptr) // cutoff if there's a new sample to start (do nothing otherwise)
                    {
                        mixData[chan].sampleIndex = 0;
                        mixData[chan].fractionalSampleIndex = 0;
                        mixData[chan].currentSample = newSample;
                        // add cumulative detune from the sample itself, and the channel tuning
                        mixData[chan].totalDetune += newSample->detune + dm.channels[chan].detune;
                        mixData[chan].freqIncrement = freqIncrement(mixData[chan].totalDetune); // compute the actual step increment
                    }
                }
                mixData[chan].kickDelay--;

                // copy in the sample, if there's more to copy
                if (mixData[chan].currentSample && mixData[chan].currentSample->len != 0)
                {
                    // float in = fast_tanh(mixData[chan].currentSample->samples[mixData[chan].sampleIndex++] * mixData[chan].gain);
                    float in = mixData[chan].currentSample->samples[mixData[chan].sampleIndex] * mixData[chan].gain;
                    
                    mixData[chan].fractionalSampleIndex += mixData[chan].freqIncrement;
                    mixData[chan].sampleIndex = mixData[chan].fractionalSampleIndex / 32768;
                    mixData[chan].currentFilter = mixData[chan].filterAlpha * mixData[chan].currentFilter + (1.0f - mixData[chan].filterAlpha) * in;
                    out += mixData[chan].currentVelocity * mixData[chan].currentFilter;

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



/* Advance to the next pattern in the sequence *without* updating the display */
void noUINextPattern(DrumMachine &dm)
{
    int pattern;
    // If we are in the sequence mode, and there is a sequence
    // then advance to the next pattern in the sequence
    if (dm.patternMode == 1 && strlen(dm.patternSequence) > 0)
    {
        pattern = dm.patternSequence[dm.patternSeqIndex];
        _setPattern(dm, pattern);
        dm.patternSeqIndex++;
        if (dm.patternSeqIndex >= strlen(dm.patternSequence))
        {
            dm.patternSeqIndex = 0;
        }
    }
}

/* Render the drum machine output to a WAV file on the SD card */
bool renderToSD(DrumMachine &dm, String fname)
{
    bool complete = false;
    int32_t len = dm.waveBufferLen;
    bool success;
    // reset the pattern index, but restore it afterwards
    int32_t oldWaveBufferIndex = dm.waveBufferIndex;
    int32_t oldPatternSeqIndex = dm.patternSeqIndex;
    String fullPath;
    fullPath = fname;
    dm.waveBufferIndex = 0;
    dm.patternSeqIndex = 0;
    // jump to the initial pattern, if in sequence mode
    if (dm.patternMode == 1)
        noUINextPattern(dm);

    success = openWAVToSD(fullPath, samplerate); // write the header
    if (!success)
    {
        Serial.println("Failed to open file for writing");
        return false;
    }

    while (!complete)
    {
        for (int i = 0; i < 4; i++)
        {
            if (dm.waveBufferIndex == 4)
                dm.waveBufferIndex = 0;
            int16_t *buffer = dm.audioBuffers[dm.waveBufferIndex];
            mix(dm);
            dm.waveBufferIndex++;

            success = appendWAVToSD(fullPath, buffer, len);

            if (!success)
            {
                Serial.println("Failed to append buffer to file");
                return false;
            }
            else
                Serial.println("Appended buffer to file");
        }
        // no sequence, we are done
        if (dm.patternMode == 0)
            complete = true;
        else
        {
            Serial.printf("PatternSeqIndex: %d\n", dm.patternSeqIndex);
            // if we are back to the start, we are done
            if (dm.patternSeqIndex == 0)
                complete = true;
            else
                // advance to the next pattern in the sequence
                noUINextPattern(dm);
        }
    }
    Serial.println("Backpatching WAV header");
    // fix the header
    success = backpatchWAVToSD(fullPath);
    if (!success)
        return false;
    // restore where we were
    dm.waveBufferIndex = oldWaveBufferIndex;
    dm.patternSeqIndex = oldPatternSeqIndex;
    mix(dm);
    return true;
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
            nextPattern(dm); // advance the pattern (only does anything in pattern sequence mode)
        }
        if (dm.waveBufferIndex > 3)
        {
            dm.waveBufferIndex = 0;
        }
        M5Cardputer.update();
    }
}