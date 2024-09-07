
#include "pattern.h"
#include "ui.h"
#include "patternui.h"


void clearPattern(DrumMachine& dm) {
    int oldStep = dm.cursor.step;
    int oldChan = dm.cursor.chan;
    for (int i = 0; i < nChans; i++) {
        for (int j = 0; j < nSteps; j++) {
            dm.cursor.step = j;
            dm.cursor.chan = i;
            setCursorChar(dm, 0);
            setCursorVel(dm, '6');
            setCursorKick(dm, 0);
        }
    }
    dm.cursor.step = oldStep;
    dm.cursor.chan = oldChan;
}



void _setPattern(DrumMachine& dm, int ix) {
    if (ix < 0 || ix >= maxPatterns)
        return;
    dm.pattern = ix;
    dm.currentPattern = &dm.allPatterns[nSteps * nChans * ix];
}



void resetPatternElement(chanData_t* elt) {
    elt->type = 0;
    elt->velocity = 6;
    elt->kickDelay = 0;
}

void setPattern(DrumMachine& dm, int ix) {
    _setPattern(dm, ix);
    updatePattern(dm);
}

void copyPattern(DrumMachine& dm) {
    int k = 0;
    for (int j = 0; j < nChans; j++) {
        for (int i = 0; i < nSteps; i++) {
            if (dm.channels[j]._enabled) // only copy enabled channels
                dm.clipboard[k] = dm.currentPattern[k];
            k++;
        }
    }
    updatePattern(dm);
}

void pastePattern(DrumMachine& dm) {
    int k = 0;
    for (int j = 0; j < nChans; j++) {
        for (int i = 0; i < nSteps; i++) {
            if (dm.channels[j]._enabled) // only paste enabled channels
                dm.currentPattern[k] = dm.clipboard[k];
            k++;
        }
    }
    updatePattern(dm);
}

void selectiveClearPattern(DrumMachine& dm) {
    int k = 0;
    for (int j = 0; j < nChans; j++) {
        for (int i = 0; i < nSteps; i++) {
            if (dm.channels[j]._enabled) // only clear enabled channels
                resetPatternElement(&dm.currentPattern[k]);
            k++;
        }
    }
    updatePattern(dm);
}





void getCursorChar(DrumMachine& dm, char &ch, int16_t &velocity)
{
    int index = dm.cursor.step + nSteps * dm.cursor.chan;
    int step = dm.currentPattern[index].type;
    
    if (step < 'a' || step > 'z')
        ch = '.';
    else
        ch = (char)step;

    velocity = dm.currentPattern[index].velocity;
}

void setCursorKick(DrumMachine& dm, int32_t kick)
{
    int index = dm.cursor.step + nSteps * dm.cursor.chan;
    dm.currentPattern[index].kickDelay = kick;
    updateMix(dm, dm.cursor.step, dm.cursor.chan);
}

void setCursorChar(DrumMachine& dm, char c)
{
    int index = dm.cursor.step + nSteps * dm.cursor.chan;
    dm.currentPattern[index].type = c;
    updateMix(dm, dm.cursor.step, dm.cursor.chan);
}

void setCursorVel(DrumMachine& dm, char c)
{
    int index = dm.cursor.step + nSteps * dm.cursor.chan;
    dm.currentPattern[index].velocity = c - '0';
    dm.liveVelocity = c - '0'; // remember this velocity so we can record in it
    updateMix(dm, dm.cursor.step, dm.cursor.chan);
}

int32_t getKickDelay(DrumMachine& dm, int16_t step, int16_t chan)
{
    int index = step + nSteps * chan;
    return dm.currentPattern[index].kickDelay;
}

sample_t *getSample(DrumMachine& dm, int16_t sampleIndex)
{
    int ix = sampleIndex - '`';
    if (ix <= 0 || ix >= 26)
        return nullptr;
    return &dm.drumSamples[ix];
}

void getStep(DrumMachine& dm, int16_t step, int16_t chan, sample_t *&sample, int16_t &velocity)
{
    int index = step + nSteps * chan;
    sample = getSample(dm, dm.currentPattern[index].type);
    velocity = dm.currentPattern[index].velocity;
}

