#include <M5Cardputer.h>
#include "audio.h"
#include "utils.h"
#include "datatypes.h"
#include "config.h"
#include "synth.h"
#include "pattern.h"
#include "ui.h"
#include "flash.h"
#include "adsr.h"

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
    dm.bufferA = dm.audioBuffers[0];
    dm.bufferB = dm.audioBuffers[1];

    dm.waveBufferIndex = 0;
}

void makeSynth(DrumMachine &dm, int index, synth_t *synth)
{
    sample_t scratchSample;

    Serial.printf("Making synth %d\n", index);
    int32_t arenaFree = getArenaFree(&dm.sampleArena);
    if (arenaFree <= 0)
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
    for (int i = 0; i < 27; i++)
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

void resetFX(fx_t *fx)
{
    // clear the retrigger state
    fx->retriggerSamples = 0;
    fx->nRetrigger = 0;
    fx->retriggerGain = 0;
    fx->retriggerGainChange = 0;
    fx->retriggerCounter = 0;
}

// reset the mix data for a single channel
void resetMixChannel(mixData_t *mx)
{
    mx->currentSample = nullptr;
    mx->sampleIndex = 0;
    mx->stepIndex = -1;
    mx->nextIndex = -1;
    mx->kickDelay = 0;
    mx->channelGain = 1.0;
    mx->totalGain = 0.0f; // will be set by the first sample
    mx->currentVelocity = 0;
    mx->currentFilter = 0.0f;
    mx->filterAlpha = 0.0f;
    mx->totalDetune = 0;
    mx->freqIncrement = 0;
    mx->fractionalSampleIndex = 0;
    mx->channelDetune = 0;
    mx->channelCutoff = 0;
    mx->smoothFreqIncrement = 32768;
    mx->reverse = false;
    resetFX(&mx->fx);
}

// called at the start of a pattern to reset the mix data
// for all channels
void resetMix(DrumMachine &dm)
{
    int chan;
    dm.mixIndex = 0;
    mixData_t *mixData = dm.mixData;
    mixData_t *mx;
    channel_t *ch;
    // reset the mix data
    for (chan = 0; chan < nChans; chan++)
    {
        mx = &mixData[chan];
        ch = &dm.channels[chan];
        resetMixChannel(mx);
        mx->channelGain = powf(1.4142, (ch->volume - 8));
        mx->channelDetune = ch->detune;
        mx->channelCutoff = ch->filterCutoff;
    }
}

void setDetuneVelocity(DrumMachine &dm, mixData_t *mx, step_t *ch)
{
    if (!mx->currentSample)
        return;
    mx->currentVelocity = ch->velocity;
    mx->portaCoeff = 1 - exp(-ch->portaTime * 2);
    // target detune state includes this step
    mx->totalDetune = mx->currentSample->adjustments.detune + ch->detune + mx->channelDetune;
    mx->totalGain = mx->channelGain * mx->currentVelocity * cBGain(mx->currentSample->adjustments.volume);

    if (mx->currentSample->adjustments.loopMode == LOOP_STRETCH) // stretch samples
        computeStretch(dm, mx, ch);
    else
        mx->freqIncrement = freqIncrement(mx->totalDetune); // compute the target step increment
    if (ch->portaTime == 0)                                 // no portamento, just set the increment
        mx->smoothFreqIncrement = mx->freqIncrement;
}

// if the sample has the loop type LOOP_STRETCH,
// compute the detune needed to make the sample
// fit the length of the pattern
// the computation is the length of the *loop* start/end stretched to the pattern
// scaled by the pitch modifier (so +1 octave = half the pattern length)
void computeStretch(DrumMachine &dm, mixData_t *mx, step_t *ch)
{
    int32_t targetSamples = dm.patternSamples;
    sample_t *sample = mx->currentSample;
    sample_adjustment_t *adj = &sample->adjustments;
    int32_t loopStart = adj->loopStart;
    int32_t loopEnd = sample->len - adj->loopEnd;
    int32_t loopSamples = loopEnd - loopStart;
    if (loopSamples <= 0) // can't stretch a sample with no loop
        return;
    float stretchFactor = (float)loopSamples / (float)targetSamples;
    float factorAdjustment = powf(2.0, mx->totalDetune / 1200.0);
    int32_t freqInc = 32768.0 * (stretchFactor * factorAdjustment);
    mx->freqIncrement = freqInc;
}

void mixTriggerSample(DrumMachine &dm, mixData_t *mx, step_t *ch)
{
    mx->stepIndex = mx->nextIndex;
    sample_t *newSample;
    int32_t index = ch->type;
    if (index == '`')
    {
        // note "continue" - change velocity and portamento target
        setDetuneVelocity(dm, mx, ch);
        return;
    }
    if (index == 'z')
    {
        // note off
        mx->loopState = LOOP_STATE_RELEASE;
        releaseADSR(&mx->adsr);
        return;
    }
    newSample = getSample(dm, index);
    if (newSample != nullptr) // cutoff if there's a new sample to start (do nothing otherwise)
    {
        mx->currentSample = newSample;
        mx->lastSample = newSample;
        setDetuneVelocity(dm, mx, ch);
        mx->loopState = LOOP_STATE_NONE;
        mx->reverse = false;

        // FX
        fx_t *fx = &mx->fx;
        resetFX(fx);

        switch (ch->fx)
        {
        case FX_REVERSE:
            mx->reverse = !mx->reverse;
            break;
        case FX_FLAM:
            fx->nRetrigger = 1;
            fx->retriggerSamples = dm.oneKickTime * 6;
            fx->retriggerGain = 1.0;
            fx->retriggerGainChange = 0.5;
            break;
        case FX_TRIPLET:
            fx->nRetrigger = 3;
            fx->retriggerSamples = dm.oneKickTime * 4;
            fx->retriggerGain = 1.0;
            fx->retriggerGainChange = 0.3;
            break;
        case FX_ROLL:
            fx->nRetrigger = 10000000;
            fx->retriggerSamples = dm.oneKickTime * 8;
            fx->retriggerGain = 1.0;
            fx->retriggerGainChange = 1.0;
            break;
        }

        mx->fractionalSampleIndex = 32768 * newSample->adjustments.trimStart;
        // gain is product of channel gain, step velocity and sample gain

        // sample cutoff is min of channel and sample cutoff
        float channelCutoff = (float)(maxFilterCutoff - mx->channelCutoff) / (float)maxFilterCutoff;
        float sampleCutoff = (1 - newSample->adjustments.cutoff / 1000.0);
        float cutoff = min(channelCutoff, sampleCutoff);
        mx->filterAlpha = iirAlpha(cutoff * cutoff); // square the cutoff for a more linear response
        // apply envelope, if one is set
        sample_adjustment_t *adj = &newSample->adjustments;

        if (adj->attackTime != 0 || adj->decayTime != 0 || adj->sustainLevel != 0 || adj->releaseTime != 0)
        {
            // / 10 to convert cB to dB
            initADSR(&mx->adsr, adj->attackTime, adj->decayTime, adj->sustainLevel / 10.0, adj->releaseTime, samplerate);
            triggerADSR(&mx->adsr);
        }
        else
        {
            mx->adsr.enabled = false;
        }
    }
}

// make sure we retrigger if we need to
void updateFX(mixData_t *mx)
{
    fx_t *fx = &mx->fx;
    if (fx->nRetrigger > 0)
    {
        fx->retriggerCounter++;
        // retrigger tripped
        if (fx->retriggerCounter >= fx->retriggerSamples)
        {
            fx->retriggerCounter = 0;
            fx->nRetrigger--;
            mx->totalGain *= fx->retriggerGain;
            if (mx->lastSample != nullptr)
            {
                mx->currentSample = mx->lastSample;
                mx->fractionalSampleIndex = 32768 * mx->currentSample->adjustments.trimStart;
            }
        }
    }
}

float mixCurrentSample(mixData_t *mx)
{
    float in = 0.0f;
    updateFX(mx);
    // copy in the sample, if there's more to copy
    if (mx->currentSample && mx->currentSample->len != 0)
    {
        int32_t trimmedEnd = mx->currentSample->len - mx->currentSample->adjustments.trimEnd;
        int32_t loopEnd = mx->currentSample->len - mx->currentSample->adjustments.loopEnd;
        mx->sampleIndex = mx->fractionalSampleIndex / 32768;
        if (mx->sampleIndex >= 0) // skip if we have a negative index due to delay in "trim"
        {
            if (mx->reverse)
                in = mx->currentSample->samples[trimmedEnd - mx->sampleIndex] * mx->totalGain;
            else
                in = mx->currentSample->samples[mx->sampleIndex] * mx->totalGain;
        }

        // envelope
        if (mx->adsr.enabled)
        {
            in *= mx->adsr.env;
            mx->adsr.env = nextADSR(&mx->adsr);
        }
        // filter
        mx->currentFilter = mx->filterAlpha * mx->currentFilter + (1.0f - mx->filterAlpha) * in;
        in = mx->currentFilter;
        // portamento
        mx->smoothFreqIncrement = mx->smoothFreqIncrement * mx->portaCoeff + mx->freqIncrement * (1 - mx->portaCoeff);
        mx->fractionalSampleIndex += (int32_t)mx->smoothFreqIncrement;

        // loop logic
        sample_adjustment_t *adj = &mx->currentSample->adjustments;
        // state NONE->LOOPING
        if (mx->sampleIndex >= adj->loopStart && adj->loopMode != LOOP_NONE && mx->loopState == LOOP_STATE_NONE)
        {
            mx->loopState = LOOP_STATE_LOOPING;
        }
        // state LOOPING -- go back
        if (mx->loopState == LOOP_STATE_LOOPING && adj->loopMode != LOOP_NONE && mx->sampleIndex >= loopEnd)
        {

            mx->sampleIndex = adj->loopStart;
            mx->fractionalSampleIndex = adj->loopStart * 32768;
        }
        // terminate sample
        if (mx->sampleIndex >= trimmedEnd)
        {
            mx->currentSample = nullptr;
        }
    }
    return in;
}

// reset the audio playback
void resetAudioPlayback(DrumMachine &dm)
{
    // stop the audio
    M5Cardputer.Speaker.end();
    // clear all the audio buffers
    for (int i = 0; i < N_BUFFERS; i++)
    {
        memset(dm.audioBuffers[i], 0, dm.maxBufferLen * sizeof(int16_t));
    }
    // reset the buffer index
    dm.waveBufferIndex = 0;
    // restart audio
    M5Cardputer.Speaker.begin();
}

// apply gain reduction, clip and write to the buffer
void writeToBuffer(float out, int16_t *buffer)
{
    float gainDrop = 32;
    // shift to a 16 bit result, clip and write
    out = out / 32;
    if (out > 32767)
        out = 32767;
    if (out < -32767)
        out = -32767;
    *buffer = out;
}

// using a constructed mixdata and channel data, mix a single channel stream into the buffer
void mixSingleSampleToBuffer(DrumMachine &dm, int16_t *buffer, int32_t len)
{
    float out;
    for (int j = 0; j < len; j++)
    {
        out = mixCurrentSample(&dm.previewData.previewMix);
        writeToBuffer(out, &buffer[j]);
        dm.mixIndex++;
    }
}

void triggerPreviewSample(DrumMachine &dm, int index)
{
    dm.previewData.previewChan = {
        .type = index,
        .velocity = 8,
        .kickDelay = 0,
        .fx = FX_NONE,
        .detune = dm.previewData.detune,
        .probability = 0,
        .portaTime = 0,
    };
    resetMixChannel(&dm.previewData.previewMix);
    mixTriggerSample(dm, &dm.previewData.previewMix, &dm.previewData.previewChan);
}

// mix (part of) a pattern into the available buffer
void mixPatternToBuffer(DrumMachine &dm, int16_t *buffer, int32_t len)
{
    int chan, j;
    mixData_t *mixData = dm.mixData;
    int32_t out;
    int32_t bufferIndex;
    int16_t newIndex = -1;
    mixData_t *mx;
    sample_adjustment_t *adj;

    for (j = 0; j < len; j++)
    {
        out = 0.0f;
        for (chan = 0; chan < nChans; chan++)
        {
            mx = &mixData[chan];

            if (!dm.channels[chan]._enabled)
                continue; // skip disabled channels

            // check if the step has changed
            newIndex = dm.mixIndex / dm.stepSamples;
            step_t *channelSteps = &dm.currentPattern[chan * nSteps]; // TODO: factor out
            if (newIndex != mx->nextIndex)
            {
                mx->nextIndex = newIndex;
                if (newIndex % 2 == 0)
                    mx->kickDelay = 0; // no swing on downbeats
                else
                    mx->kickDelay = dm.stepSamples * dm.swing / 200;                     // apply swing (max=50% delay)
                mx->kickDelay += channelSteps[mx->nextIndex].kickDelay * dm.oneKickTime; // add on forced kick delay in the channel
            }
            // did it change yet? if so, we need a new sample
            if (mx->kickDelay == 0)
                mixTriggerSample(dm, mx, &channelSteps[mx->nextIndex]);
            out += mixCurrentSample(mx);
            mx->kickDelay--;
        }
        writeToBuffer(out, &buffer[j]);
        dm.mixIndex++;
    }

    // apply FX
    dynamicCompressFX(buffer, len, &dm.mixFX.compressor);
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
            mixPatternToBuffer(dm, dm.bufferB, dm.waveBufferLen);
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

bool feedBuffers(DrumMachine &dm)
{
    if (M5Cardputer.Speaker.isPlaying(0) != 2)
    {
        M5Cardputer.Speaker.playRaw(dm.bufferA, dm.waveBufferLen, samplerate, false, 1, 0);
        int16_t *tmp_buffer;
        // cycle buffers
        tmp_buffer = dm.bufferA;
        dm.bufferA = dm.bufferB;
        dm.bufferB = tmp_buffer;
        M5Cardputer.update();
        return true;
    }
    return false;
}

// simple, just keep feeding the buffers
void feedPreviewBuffers(DrumMachine &dm)
{
    while (feedBuffers(dm))
    {
        mixSingleSampleToBuffer(dm, dm.bufferB, dm.waveBufferLen);
    }
}

// feed the audio buffers with the mixed audio
void feedPatternBuffers(DrumMachine &dm)
{
    // room in the queue? stuff the buffers
    while (feedBuffers(dm))
    {
        mixPatternToBuffer(dm, dm.bufferB, dm.waveBufferLen);
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
    }
}

// recalculate the BPM and related values
// including the length of the audio buffers
void recalcBPM(DrumMachine &dm)
{
    dm.stepSamples = samplerate * 60 / (dm.bpm * 4);
    dm.oneKickTime = dm.stepSamples / kickSubdiv;
    dm.patternSamples = dm.stepSamples * nSteps;
    dm.waveBufferLen = dm.patternSamples / bufferBeats;
}