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


void allocateMix(DrumMachine &dm)
{
    int32_t bufSamples;
    int32_t maxPatSamples;
    maxPatSamples = nSteps * samplerate * 60 / (minBPM * 4);
    bufSamples = maxPatSamples / bufferBeats;
    for (int i = 0; i < N_BUFFERS; i++)
        dm.audioBuffers[i] = (int16_t *)allocBuffer(dm.audioBuffers[i], bufSamples, sizeof(int16_t));
    dm.maxBufferLen = bufSamples;
    dm.scratchBuffer = dm.audioBuffers[0]; // TODO: make end of arena?
    dm.bufferA = dm.audioBuffers[0];
    dm.bufferB = dm.audioBuffers[1];

    dm.waveBufferIndex = 0;
}

void makeSynth(DrumMachine &dm, int index, synth_t *synth)
{
    sample_t scratchSample;
    
    Serial.printf("Making synth %d\n", index);
    int32_t arenaFree = getArenaFree(&dm.sampleArena);
    if(arenaFree<=0)
    {
        Serial.println("Out of memory");
        dm.drumSamples[index].samples = nullptr;
        dm.drumSamples[index].len = 0;
        return;
    }
    scratchSample.samples = (int16_t *)getArenaTop(&dm.sampleArena);    
    scratchSample.len = arenaFree / sizeof(int16_t) - 1;
    scratchSample.len = min(scratchSample.len, (int32_t)(samplerate * maxSampleLen)); // limit max sample size    
    createSynth(&scratchSample, samplerate, synth);
    int32_t trueLen = findSampleEnd(scratchSample.samples, scratchSample.len, 100, 0.03);    
    scratchSample.len = trueLen;    
    // adjust arena top
    setArenaTop(&dm.sampleArena, scratchSample.samples + scratchSample.len);
    dm.drumSamples[index] = scratchSample;
}

void clearSamples(DrumMachine &dm)
{
    clearArena(&dm.sampleArena);
    for(int i = 0; i < 27; i++)
    {
        dm.drumSamples[i].samples = nullptr;
        dm.drumSamples[i].len = 0;
    }
}

void synthKitSamples(DrumMachine &dm, kit_t &kit)
{

    clearSamples(dm);
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
    int32_t totalDetune = preview->adjustments.detune;
    int32_t freqInc = freqIncrement(totalDetune);
    int32_t sampleIndex = 0;
    int32_t fractionalSampleIndex = freqInc * preview->adjustments.trimStart;
    int32_t len = preview->len;
    int32_t end = len - preview->adjustments.trimEnd;
    int32_t out;
    float sampleCutoff = 1 - (preview->adjustments.cutoff / 1000.0);
    float alpha = iirAlpha(sampleCutoff * sampleCutoff);
    float filter = 0.0f;
    float gain = cBGain(preview->adjustments.volume);
    for (int i = 0; i < dm.waveBufferLen; i++)
    {
        if (sampleIndex >= len || sampleIndex >= end || sampleIndex < 0)
        {
            dm.audioBuffers[0][i] = 0;            
            continue;
        }
        float in = preview->samples[sampleIndex] * gain;
        fractionalSampleIndex += freqInc;
        sampleIndex = fractionalSampleIndex / 32768;
        filter = alpha * filter + (1.0f - alpha) * in;
        out = filter;
        if (out > 32767)
            out = 32767;
        if (out < -32767)
            out = -32767;
        dm.audioBuffers[0][i] = out;
    }
    M5Cardputer.Speaker.playRaw(dm.audioBuffers[0], len, samplerate, false, 1, 0);
}

void resetMix(DrumMachine &dm)
{
    int chan;
    dm.mixIndex = 0;
    mixData_t *mixData = dm.mixData;
    // reset the mix data
    for (chan = 0; chan < nChans; chan++)
    {
        mixData[chan].currentSample = nullptr;
        mixData[chan].sampleIndex = 0;
        mixData[chan].stepIndex = -1;
        mixData[chan].nextIndex = -1;
        mixData[chan].kickDelay = 0;
        mixData[chan].channelGain = powf(1.4142, (dm.channels[chan].volume - 8));
        mixData[chan].totalGain = 0.0f; // will be set by the first sample
        mixData[chan].currentVelocity = 0;
        mixData[chan].currentFilter = 0.0f;
        mixData[chan].filterAlpha = 0.0f;

        
    }
}

void mixPatternToBuffer(DrumMachine &dm, int16_t *buffer)
{
    int chan, j;
    mixData_t *mixData = dm.mixData;
    int32_t out;
    int32_t gainDrop = 32;
    int32_t bufferIndex;
    int16_t newIndex = -1;
    buffer = dm.bufferB;

    for (j = 0; j < dm.waveBufferLen; j++)
    {
        out = 0;
        for (chan = 0; chan < nChans; chan++)
        {
            if (!dm.channels[chan]._enabled)
                continue; // skip disabled channels
            newIndex = dm.mixIndex / dm.stepSamples;
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
                sample_t *newSample;
                getStep(dm, mixData[chan].stepIndex, chan, newSample, mixData[chan].currentVelocity);
                getDetune(dm, mixData[chan].stepIndex, chan, mixData[chan].totalDetune);
                if (newSample != nullptr) // cutoff if there's a new sample to start (do nothing otherwise)
                {
                    mixData[chan].sampleIndex = 0;
                    
                    mixData[chan].currentSample = newSample;
                    // add cumulative detune from the sample itself, and the channel tuning
                    mixData[chan].totalDetune += newSample->adjustments.detune + dm.channels[chan].detune;
                    mixData[chan].freqIncrement = freqIncrement(mixData[chan].totalDetune); // compute the actual step increment
                    mixData[chan].fractionalSampleIndex = mixData[chan].freqIncrement * newSample->adjustments.trimStart; 
                    // gain is product of channel gain, step velocity and sample gain
                    mixData[chan].totalGain = mixData[chan].channelGain * mixData[chan].currentVelocity * cBGain(newSample->adjustments.volume);

                    // sample cutoff is min of channel and sample cutoff
                    float channelCutoff = (float)(maxFilterCutoff - dm.channels[chan].filterCutoff) / (float)maxFilterCutoff;
                    float sampleCutoff = (1 - newSample->adjustments.cutoff / 1000.0);
                    
                    float cutoff = min(channelCutoff, sampleCutoff);
                    mixData[chan].filterAlpha = iirAlpha(cutoff*cutoff); // square the cutoff for a more linear response
                }
            }
            mixData[chan].kickDelay--;

            // copy in the sample, if there's more to copy
            if (mixData[chan].currentSample && mixData[chan].currentSample->len != 0)
            {                            
                mixData[chan].fractionalSampleIndex += mixData[chan].freqIncrement;
                mixData[chan].sampleIndex = mixData[chan].fractionalSampleIndex / 32768;
                float in = 0.0f;
                if(mixData[chan].sampleIndex >= 0) // skip if we have a negative index (delayed start)                
                    in = mixData[chan].currentSample->samples[mixData[chan].sampleIndex] * mixData[chan].totalGain;
                mixData[chan].currentFilter = mixData[chan].filterAlpha * mixData[chan].currentFilter + (1.0f - mixData[chan].filterAlpha) * in;

                out +=  mixData[chan].currentFilter;
                // overran the sample, so stop
                int32_t trimmedEnd = mixData[chan].currentSample->len - mixData[chan].currentSample->adjustments.trimEnd;
                if (mixData[chan].sampleIndex >= mixData[chan].currentSample->len || mixData[chan].sampleIndex >= trimmedEnd)
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
        dm.mixIndex++;
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
        Serial.println("Failed to open render WAV file for writing");
        return false;
    }
    while (!complete)
    {
        resetMix(dm);
        for (int i = 0; i < bufferBeats; i++)
        {
            if (dm.waveBufferIndex == bufferBeats)
                dm.waveBufferIndex = 0;
            mixPatternToBuffer(dm, dm.bufferB);
            dm.waveBufferIndex++;
            success = appendWAVToSD(fullPath, dm.bufferB, dm.waveBufferLen);
            if (!success)
            {
                return false;
            }
        }
        // no sequence, we are done
        if (dm.patternMode == 0)
            complete = true;
        else
        {
            // if we are back to the start, we are done
            if (dm.patternSeqIndex == 0)
                complete = true;
            else
                // advance to the next pattern in the sequence
                noUINextPattern(dm);
        }
    }
    // fix the header
    success = backpatchWAVToSD(fullPath);
    if (!success)
        return false;
    // restore where we were
    dm.waveBufferIndex = 0;
    resetMix(dm);
    dm.patternSeqIndex = oldPatternSeqIndex;
    return true;
}

// feed the audio buffers with the mixed audio
void feedPatternBuffers(DrumMachine &dm)
{
    // room in the queue? stuff the buffers
    while (M5Cardputer.Speaker.isPlaying(0) != 2)
    {
        M5Cardputer.Speaker.playRaw(dm.bufferA, dm.waveBufferLen, samplerate, false, 1, 0);

        int16_t *tmp_buffer;
        // cycle buffers
        tmp_buffer = dm.bufferA;
        dm.bufferA = dm.bufferB;
        dm.bufferB = tmp_buffer;

        mixPatternToBuffer(dm, dm.bufferB);
        dm.waveBufferIndex++;
        if (dm.waveBufferIndex == 2)
        {
            if (dm.syncMix) // mix in time for the next buffer change
            {
                dm.syncMix = 0;
            }
            dm.syncMillis = millis();
        }
        if (dm.waveBufferIndex >= bufferBeats)
        {
            dm.waveBufferIndex = 0;
            nextPattern(dm); // advance the pattern (only does anything in pattern sequence mode)
            resetMix(dm);
        }
        M5Cardputer.update();
    }
}

void recalcBPM(DrumMachine &dm)
{
    dm.stepSamples = samplerate * 60 / (dm.bpm * 4);
    dm.oneKickTime = dm.stepSamples / kickSubdiv;
    dm.patternSamples = dm.stepSamples * nSteps;
    dm.waveBufferLen = dm.patternSamples / bufferBeats;
}