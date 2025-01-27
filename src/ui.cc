#include "audio.h"
#include "pattern.h"
#include "channels.h"
#include "patternui.h"
#include "previewui.h"
#include "kits.h"
#include "synth.h"
#include "flash.h"
#include "serialize.h"
#include "helpui.h"
#include "config.h"
#include "ui.h"
#include <LittleFS.h>

/* Render the whole pattern (or pattern sequence) to
the next available WAV file on the SD card */
bool renderPattern(DrumMachine &dm)
{
    String fname = findFreeRenderFilenameSD();
    Serial.println("Rendering to " + fname);

    // get just the substring after /
    String justName = fname.substring(fname.lastIndexOf("/") + 1);
    String msg = "->" + justName;

    lowerMessage(msg.c_str());
    bool success = renderToSD(dm, fname);

    if (!success)
    {
        msg = "Failed to render";
        lowerMessage(msg.c_str());
        return false;
    }
    updatePattern(dm);
    return true;
}

void updatePattern(DrumMachine &dm)
{
    recalcBPM(dm);
    recalcChannels(dm);
    if (!dm.splashFlag)
    {
        setGraphicsModePattern();
        redrawPattern(dm);
        drawStatus(dm);
    }
    requestMix(dm);
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

void reinitState(DrumMachine &dm)
{
    dm.patternCursor = 0;
    dm.patternSeqIndex = 0;
    dm.patternMode = 0;
    _setPattern(dm, dm.pattern); // set the pattern pointer
    updatePattern(dm);           // update the pattern / redraw
}

// load a drum machine state from a file
bool loadDrumMachine(DrumMachine &dm, String &fname)
{
    bool success;

    File ser = LittleFS.open(fname, "r");
    if (!ser)
    {
        Serial.println("Failed to open file for reading");
        return false;
    }
    int oldKit = dm.kit;
    success = readDrumMachine(dm, ser);
    ser.close();
    if (!success)
    {
        Serial.println("Failed to deserialize");

        return false;
    }
    reinitState(dm);
    if (oldKit != dm.kit)
    {
        oldKit = dm.kit;
        dm.kit = -1;
        setKit(dm, oldKit);
    }

    if (dm.splashFlag)
    {
        dm.splashFlag = 0;
        updatePattern(dm);
    }
    return true;
}

void initState(DrumMachine &dm)
{
    Serial.println("Initializing drum machine state");
    // Allocate mix buffers
    createArena(&dm.sampleArena);
    allocateMix(dm);
    Serial.println("Allocated mix buffers");

    // print free heap space

    dm.kit = -1;          // set to -1 so we always set the kit
                          // Create the drum samples
    dm.nKits = nDrumKits; // initialize the number of kits (defined in kits.h)
    dm.splashFlag = true;
}

void resetGlobalFX(DrumMachine &dm)
{
    dm.mixFX.compressor.threshold_dB = -16;
    dm.mixFX.compressor.ratio = 4;
    dm.mixFX.compressor.attack_ms = 1;
    dm.mixFX.compressor.release_ms = 100;
    dm.mixFX.compressor.makeup_dB = 3;
    dm.mixFX.compressor.sampleRate = samplerate;
    dm.mixFX.compressor.rms = 0;    
}

// reset the state
void resetState(DrumMachine &dm)
{
    dm.lastMessageMillis = 0;

    dm.cursor.width = 12;
    dm.cursor.height = 12;

    // reset the mode/action
    dm.lastMode = PLAY_MODE_PATTERN;
    dm.bankAction = ACTION_NONE;

    // Set global bpm/swing
    dm.bpm = 120;
    dm.swing = 0;
    dm.syncMillis = millis();
    dm.beatTime = 0;
    dm.volume = 16;
    dm.liveMode = 0;
    dm.liveVelocity = 6;
    dm.playStep = 0.0;

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

    resetGlobalFX(dm);

    // Recalculate BPM settings and channel configurations
    recalcBPM(dm);
    recalcChannels(dm);

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

// serialise *just* the sample adjustments
bool saveSampleAdjustments(DrumMachine &dm, int32_t bank)
{
    char filename[128];
    sprintf(filename, "/kit-%04d.adj", bank);

    File ser = LittleFS.open(filename, "w");
    if (!ser)
    {
        Serial.println("Failed to open file for writing");
        return false;
    }
    serialiseKit(dm, ser);
    ser.close();
    return true;
}


bool resetSampleAdjustments(DrumMachine &dm)
{
    int32_t i;
    for(i=0; i<maxSamples; i++)
    {
        dm.drumSamples[i].adjustments.detune = 0;
        dm.drumSamples[i].adjustments.volume = 0;
        dm.drumSamples[i].adjustments.cutoff = 0;
        dm.drumSamples[i].adjustments.trimStart = 0;
        dm.drumSamples[i].adjustments.trimEnd = 0;
        dm.drumSamples[i].adjustments.loopStart = 0;
        dm.drumSamples[i].adjustments.loopEnd = 0;
        dm.drumSamples[i].adjustments.loopMode = LOOP_NONE;
        dm.drumSamples[i].adjustments.attackTime = 0;
        dm.drumSamples[i].adjustments.decayTime = 0;
        dm.drumSamples[i].adjustments.sustainLevel = 0;
        dm.drumSamples[i].adjustments.releaseTime = 0;
        dm.drumSamples[i].adjustments.samplePtr = 0;
    }
}

bool loadSampleAdjustments(DrumMachine &dm, int32_t bank)
{
    char filename[128];
    sprintf(filename, "/kit-%04d.adj", bank);

    File ser = LittleFS.open(filename, "r");
    if (!ser)
    {
        Serial.println("Failed to open file for reading");
        return false;
    }

    bool success = deserialiseKit(dm, ser);
    ser.close();

    if(!success)
    {
        Serial.println("Failed to deserialise kit");
        resetSampleAdjustments(dm);
        return false;
    }

    
    return true;
}

// save the current drum machine state to a file
bool saveDrumMachine(DrumMachine &dm, String &fname)
{
    Serial.println("Saving drum machine state to " + fname);
    File ser = LittleFS.open(fname, "w");
    if (!ser)
    {
        Serial.println("Failed to open file for writing");
        return false;
    }
    writeDrumMachine(dm, ser);
    ser.close();
    return true;
}

// Set the current kit to the given index
// resynthetizes all samples
void setKit(DrumMachine &dm, int kit)
{
    if (kit < 0)
        return;
    int oldKit = dm.kit;
    dm.kit = kit;
    // is this a base kit? resynth the samples
    if (dm.kit < nDrumKits)
    {
        synthKitSamples(dm, drumKits[dm.kit]);
        //loadSampleAdjustments(dm, dm.kit); // load the sample adjustments (if any)
        requestMix(dm);

        return;
    }

    // try loading a cached kit from the SD card
    char kitName[128];
    snprintf(kitName, 127, "%s/base-%c.kit", basePathKits.c_str(), alphaNumericChars[kit+1]);
    Serial.println("Loading kit " + String(kitName));

    bool success = loadKitSD(String(kitName), dm);
    if (!success)
    {
        if (dm.kit != oldKit)
        {
            setKit(dm, oldKit);
        }
    }
    else
    {
        //loadSampleAdjustments(dm, dm.kit); // load the sample adjustments (if any)
        requestMix(dm);
    }
}

void updateMix(DrumMachine &dm, int16_t step, int16_t chan)
{
    // for now, just mix the whole pattern
    dm.syncMix = 1; // flag to update on next loop
}

void requestMix(DrumMachine &dm)
{
    dm.syncMix = 1; // flag to update on next loop
}

// in patternMode=0 do nothing;
// in patternMode=1 advance to next pattern in sequence
// wrapping around to the start if necessary
// if fill is set, jump to the fill, then return to the sequence
void nextPattern(DrumMachine &dm)
{
    int pattern;

    // Is it time change to the new pattern mode?
    if (dm.patternModeSwitch)
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
        drawTopLine(dm);
    }

    if (dm.fillPattern > 0)
    {
        // positive, switch to the fill pattern
        dm.lastPatternBeforeFill = dm.pattern;
        pattern = dm.fillPattern;
        dm.fillPattern = -1;
        setPattern(dm, pattern);
        updatePattern(dm);
        return;
    }

    // If we are in the sequence mode, and there is a sequence
    // then advance to the next pattern in the sequence
    if (dm.patternMode == 1 && strlen(dm.patternSequence) > 0)
    {
        pattern = dm.patternSequence[dm.patternSeqIndex];
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
        // restore where we were before the fill
        if (dm.lastPatternBeforeFill > 0)
        {
            pattern = dm.lastPatternBeforeFill;
            dm.lastPatternBeforeFill = 0;
            setPattern(dm, pattern);
            updatePattern(dm);
        }
    }
}

void updateUI(DrumMachine &dm)
{
    if (dm.playMode == PLAY_MODE_PATTERN)
        patternModeUpdate(dm);
    if (dm.playMode == PLAY_MODE_PREVIEW)
        previewModeUpdate(dm);
    if (dm.playMode == PLAY_MODE_HELP)
        helpModeUpdate(dm);
}

void setPlayMode(DrumMachine &dm, int mode)
{
    if (mode == dm.playMode)
        return; // already in the mode, don't do anything!

    dm.lastMode = dm.playMode;
    dm.playMode = mode;
    resetAudioPlayback(dm);
    if (mode == PLAY_MODE_PATTERN)
    {
        updatePattern(dm);
        // force a mix
        feedPatternBuffers(dm);
    }
    if (mode == PLAY_MODE_PREVIEW)
    {

        initPreviewMode(dm);
    }
    if (mode == PLAY_MODE_HELP)
    {
        setGraphicsModeHelp(dm);
    }
}
