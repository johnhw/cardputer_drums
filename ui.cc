#include "ui.h"
#include "audio.h"
#include "pattern.h"
#include "channels.h"
#include "patternui.h"
#include "previewui.h"
#include "kits.h"
#include "synth.h"

void updatePattern(DrumMachine& dm) {
    setGraphicsModePattern();
    redrawPattern(dm);
    drawStatus(dm);
    requestMix(dm);
}

void recalcBPM(DrumMachine& dm) {
    dm.stepSamples = samplerate * 60 / (dm.bpm * 4);
    dm.oneKickTime = dm.stepSamples / kickSubdiv;
    dm.patternSamples = dm.stepSamples * nSteps;
    dm.waveBufferLen = dm.patternSamples / 4;
}

void initCursor(cursor_t &cursor)
{
    // Initialize cursor state
    cursor.step = 0;
    cursor.chan = 0;
    cursor.flash = 0;
    cursor.on = 0;
    cursor.dirty = 0;
}


void resetState(DrumMachine& dm)
{
    dm.cursor.width = 12;
    dm.cursor.height = 12;

    // Set global bpm/swing
    dm.bpm = 120;
    dm.swing = 0;
    dm.syncMillis = millis();
    dm.beatTime = 0;
    dm.volume = 16;

    strcpy(dm.patternSequence, ""); // reset the pattern sequence
    dm.patternCursor = 0;
    dm.patternSeqIndex = 0;
    dm.patternMode = 0;

    // Reset all channels
    for (int chan = 0; chan < nChans; chan++)
    {
        dm.channels[chan].volume = 100;
        dm.channels[chan].mute = 0;
        dm.channels[chan].solo = 0;
        dm.channels[chan]._enabled = 1;
    }

    // Allocate mix buffers
    allocateMix(dm);

    // Recalculate BPM settings and channel configurations
    recalcBPM(dm);
    recalcChannels(dm);

    // Create the drum samples
    dm.kit = 1;
    createSamples(dm, drumKits[dm.kit]);

    // Clear all patterns
    for (int i = 0; i < maxPatterns; i++)
    {
        _setPattern(dm, i);
        clearPattern(dm);
    }

    // Set the initial pattern
    setPattern(dm, 1);

    initCursor(dm.cursor);
    
    // Draw the initial cursor position
    drawCursor(dm, 1);

    // Update the pattern
    updatePattern(dm);
}




void updateMix(DrumMachine& dm, int16_t step, int16_t chan)
{
    // for now, just mix the whole pattern
    // mix();
    dm.syncMix = 1; // flag to update on next loop
}

void requestMix(DrumMachine& dm)
{
    dm.syncMix = 1; // flag to update on next loop
}



void setPlayMode(DrumMachine &dm, int mode)
{
  dm.playMode = mode;
  if (mode == 0)
  {
    updatePattern(dm);
  }
  if (mode == 1)
  {
    setGraphicsModePreview();
  }
}
