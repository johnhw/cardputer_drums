#ifndef DATATYPES_H
#define DATATYPES_H
#include <stdint.h>
#include "config.h"

// one channel
typedef struct channel_t
{
  float volume; // 0.0 to 1.0 (but greater can be used for overdrive)
  int filterCutoff; // 0-16 16=no filter, 0=full filter, 1/16th of samplerate
  int8_t mute;
  int8_t solo;
  int8_t _enabled; // NB: not directly modified; updated to reflect mute/solo status of all channels

} channel_t;

// a sample, with PCM data, length and a frequency
typedef struct sample_t
{
  int16_t *samples = 0;
  int32_t len = 0;
  int32_t freqIncrement = 0; // 32768 = 1.0
} sample_t;

// one step of a channel
typedef struct chanData_t
{
  int16_t type;
  int16_t velocity;
  int32_t kickDelay; // additional delay, in kickSubdiv units
} chanData_t;

// the cursor location/flash state
typedef struct cursor_t
{
  int8_t step;
  int8_t chan;
  int8_t width;
  int8_t height;
  int16_t flash;
  int8_t on;
  int8_t dirty;
} cursor_t;

// data for mixing one channel into the final mix
typedef struct mixData_t
{
  sample_t *currentSample;
  int32_t sampleIndex;
  int16_t stepIndex;
  int16_t nextIndex;
  int32_t kickDelay;
  int16_t currentVelocity;
  float currentFilter;  
  float filterAlpha;
} mixData_t;

struct DrumMachine {
    sample_t drumSamples[26];
    int16_t* audioBuffers[4] = {nullptr, nullptr, nullptr, nullptr}; // 4 buffers
    int8_t waveBufferIndex = 0;
    int32_t patternSamples; // number of samples in a whole pattern
    int16_t waveBufferLen;  // samples in one buffer (1/4 pattern)
    int8_t playMode = 0;    // 0=normal; 1=sample preview
    int16_t syncMix = 0;    // flag to indicate to remix at the next pattern loop
    int16_t stepSamples;    // length of one step in samples (usually 1/16th of a pattern)
    int32_t oneKickTime;    // time in samples to kick forward by one kickSubdiv of a step (usually 1/12 of a step)
    uint32_t syncMillis;    // time in milliseconds of the last pattern loop start
    int16_t beatTime;       // the current beat/step we are in right now
    int16_t bpm = 120;
    int16_t swing = 0;
    int16_t pattern = 0;
    int16_t kit = 0;
    int16_t volume = 0; // 16 volume levels
    char patternSequence[maxPatternSequence];
    int8_t patternCursor = 0;
    int8_t patternSeqIndex = 0;
    int8_t patternMode = 0; // 0 = one pattern, 1 = sequence
    int8_t patternModeSwitch = 0; // set to indicate that the pattern should switch at the next mix!
    int16_t nKits; // number of kits available (set at start)
    int16_t lastMode; // last mode we were in (so we can return to it)
    cursor_t cursor;
    channel_t channels[nChans];
    chanData_t* currentPattern;
    chanData_t allPatterns[nSteps * nChans * maxPatterns];
    chanData_t clipboard[nSteps * nChans];
};





#endif

