#include "ui.h"
#include "audio.h"
#include "pattern.h"
#include "channels.h"
#include "patternui.h"
#include "previewui.h"
#include "confirmui.h"
#include "kits.h"
#include "synth.h"
#include "flash.h"
#include "serialize.h"

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


// save the current drum machine state to a file
bool saveDrumMachine(DrumMachine &dm, String &fname) { 
    String json;
    serializeDrumMachine(dm, json);
    return saveFile(basePath + "/" + fname, json); 
}

// load a drum machine state from a file
bool loadDrumMachine(DrumMachine &dm, String &fname) {
    String json;
    bool success;
    if (!loadFile(basePath + "/" + fname, json)) {
        return false;
    }
    
    // we do this twice: first time to check we *can*
    // deserialize; then we reset the state and do it again
    // otherwise a failed load will leave the machine in a bad state
    success = deserializeDrumMachine(dm, json);
    if(!success) {
        return false;
    }
    resetState(dm); // reset the machine
    success = deserializeDrumMachine(dm, json);
    if(!success) {
        return false; // this should never happen!
    }
    _setPattern(dm, dm.pattern); // set the pattern pointer
    updatePattern(dm); // update the pattern / redraw
    setKit(dm, dm.kit); // set the kit (if it's changed)
    return true;
}

void initState(DrumMachine &dm)
{
    // Allocate mix buffers
    allocateMix(dm);
    dm.kit = -1; // set to -1 so we always set the kit
      // Create the drum samples
    dm.nKits = nDrumKits; // initialize the number of kits (defined in kits.h)
    setKit(dm, 0); // set the default kit (note resetState doesn't do this because it's very slow)
}

// reset the state 
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
        dm.channels[chan].filterCutoff = 0;
        dm.channels[chan]._enabled = 1;
    }

    
    // Recalculate BPM settings and channel configurations
    recalcBPM(dm);
    recalcChannels(dm);


    // Clear all patterns
    for (int i = 0; i < maxPatterns; i++)
    {
        _setPattern(dm, i);
        clearPattern(dm);
    }
    initState(dm);

    // Set the initial pattern
    setPattern(dm, 1);

    initCursor(dm.cursor);
    
    // Draw the initial cursor position
    drawCursor(dm, 1);

 
    // Update the pattern
    updatePattern(dm);
}

// Set the current kit to the given index
// resynthetizes all samples
void setKit(DrumMachine& dm, int kit)
{
    if (kit < 0 || kit >= dm.nKits)
         return;
    dm.kit = kit;
    createSamples(dm, drumKits[dm.kit]);
    requestMix(dm);
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


// in patternMode=0 do nothing;
// in patternMode=1 advance to next pattern in sequence
// wrapping around to the start if necessary
void nextPattern(DrumMachine &dm)
{
    int pattern;

    if(dm.patternModeSwitch)
    {
      dm.patternModeSwitch = 0; // reset flag      
      if (dm.patternMode == 0)
      {

        dm.patternMode = 1;
        dm.patternSeqIndex = 0;
      }
      else
      {
        dm.patternMode = 0;
      }
    }


    if (dm.patternMode == 1 && strlen(dm.patternSequence) > 0)
    {
        
        pattern = dm.patternSequence[dm.patternSeqIndex] ;

        
        setPattern(dm, pattern);        
        updatePattern(dm);
        dm.patternSeqIndex++;
        if (dm.patternSeqIndex >= strlen(dm.patternSequence))
        {
            dm.patternSeqIndex = 0;
        }
    }
    else
    {
        // do nothing
    }
}

void updateUI(DrumMachine& dm)
{
    if (dm.playMode == PLAY_MODE_PATTERN)
        patternModeUpdate(dm);
    if (dm.playMode == PLAY_MODE_PREVIEW)
        previewModeUpdate(dm);
    if (dm.playMode == PLAY_MODE_CONFIRM)
        confirmModeUpdate(dm);

}

void setPlayMode(DrumMachine &dm, int mode)
{
    dm.lastMode = dm.playMode;
    dm.playMode = mode;  
  if (mode == PLAY_MODE_PATTERN)
  {
    updatePattern(dm);
  }
  if (mode == PLAY_MODE_PREVIEW)
  {
    setGraphicsModePreview();
  }
  if (mode == PLAY_MODE_CONFIRM)
  {
    setGraphicsModeConfirm();
  }
  
}
