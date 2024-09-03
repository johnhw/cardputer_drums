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
#include "helpui.h"

#include <LittleFS.h>

void updatePattern(DrumMachine& dm) {
    recalcBPM(dm);
    recalcChannels(dm);
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
        
    File ser = LittleFS.open(basePathPattern + "/" + fname, "w");
    if (!ser) {
        Serial.println("Failed to open file for writing");
        return false;
    }
    writeDrumMachine(dm, ser);
    ser.close();    
    return true;
}


void reinitState(DrumMachine &dm)
{
    dm.patternCursor = 0;
    dm.patternSeqIndex = 0;
    dm.patternMode = 0;
    _setPattern(dm, dm.pattern); // set the pattern pointer
    updatePattern(dm); // update the pattern / redraw
}

// load a drum machine state from a file
bool loadDrumMachine(DrumMachine &dm, String &fname) {
        
    bool success;
    File ser = LittleFS.open(basePathPattern + "/" + fname, "r");
    if (!ser) {
        Serial.println("Failed to open file for reading");
        return false;
    }
    int oldKit = dm.kit;
    success = readDrumMachine(dm, ser);
    ser.close();
    if(!success) {
        Serial.println("Failed to deserialize");
        return false;
    }
    
    reinitState(dm);
    if(oldKit != dm.kit)
    {
        oldKit = dm.kit;
        dm.kit = -1;
        setKit(dm, oldKit); 
    }
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
        dm.channels[chan].volume = 8;
        dm.channels[chan].mute = 0;
        dm.channels[chan].solo = 0;
        dm.channels[chan].filterCutoff = 0;
        dm.channels[chan]._enabled = 1;
        
    }

    
    // Recalculate BPM settings and channel configurations
    recalcBPM(dm);
    recalcChannels(dm);

    initState(dm);
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

// Set the current kit to the given index
// resynthetizes all samples
void setKit(DrumMachine& dm, int kit)
{
    if (kit < 0 || kit >= dm.nKits || dm.kit == kit)
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
    if (dm.playMode == PLAY_MODE_HELP)
        helpModeUpdate(dm);
        

}

void setPlayMode(DrumMachine &dm, int mode)
{
  if(mode == dm.playMode)
    return; // already in the mode, don't do anything!

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
  if(mode == PLAY_MODE_HELP)
  {
    setGraphicsModeHelp(dm);
  }
  
}
