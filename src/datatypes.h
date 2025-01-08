#ifndef DATATYPES_H
#define DATATYPES_H
#include <stdint.h>
#include "config.h"

// one channel
typedef struct channel_t
{
  float volume; // 0 - 16, 8 = 100%, each step is +/- 3dB
  int filterCutoff; // 0-16 16=no filter, 0=full filter, 1/16th of samplerate
  int8_t mute;
  int8_t solo;
  int8_t _enabled; // NB: not directly modified; updated to reflect mute/solo status of all channels
  int32_t detune; // detune in cents
} channel_t;

typedef struct sample_adjustment_t
{
  int32_t detune = 0; // detune in cents
  int32_t cutoff = 0; 
  int32_t volume = 0; // signed, each step is 3dB
  int32_t trimStart = 0; // start of the sample (negative = delay)
  int32_t trimEnd = 0; // offset from end of the sample (always positive)
  int32_t loopStart = 0; // start of the loop 
  int32_t loopEnd = 0; // end of the loop (if start==end, no loop)
  // envelope (TODO)
  int32_t attackTime = 0; // attack time in ms
  int32_t decayTime = 0; // decay time in ms
  int32_t sustainLevel = 0; // sustain level in dB
  int32_t releaseTime = 0; // release time in ms
} sample_adjustment_t;

// a sample, with PCM data, length and a frequency
typedef struct sample_t
{
  int16_t *samples = 0;
  int32_t len = 0;    
  sample_adjustment_t adjustments;
} sample_t;

enum FX {
  FX_NONE=0,
  FX_FLAM=1,
  FX_ROLL=2,
  FX_REVERSE=3,  
};

// one step of a channel
typedef struct chanData_t
{
  int16_t type;
  int16_t velocity;
  int32_t kickDelay; // additional delay, in kickSubdiv units
  int8_t fx; // effect to apply
  int32_t detune; // detune in cents
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

// memory arena for samples
typedef struct arena_t {
    void* start;
    void* end;    
    void *top;
} arena_t;



// data for mixing one channel into the final mix
typedef struct mixData_t
{
  sample_t *currentSample;  
  int32_t sampleIndex;
  int32_t fractionalSampleIndex;
  int16_t stepIndex;
  int16_t nextIndex;
  int32_t kickDelay;
  int16_t currentVelocity;
  float gain;
  float currentFilter;  
  float filterAlpha;
  int32_t totalDetune;
  int32_t freqIncrement;
} mixData_t;

typedef struct previewData_t
{
  char lastSample = 0;
  int32_t lastDetune = 0;
  int16_t currentParam = 0; // current parameter being edited in preview mode
  bool previewDirty = true;
} previewData_t;

struct DrumMachine {
    sample_t drumSamples[27];        
    int16_t* audioBuffers[N_BUFFERS]; // double buffering
    int16_t *scratchBuffer; // points to audioBuffers[0]
    int16_t *bufferA, *bufferB; // the two buffers
    arena_t sampleArena; // memory arena for samples

    int32_t mixIndex = 0; // current index inside the pattern (in samples)
    int8_t waveBufferIndex = 0; // current buffer count (4 buffers per bar)
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
    float playStep = 0; // the (fractional) step we are currently playing
    int64_t tapBufferMillis[4] = {0, 0, 0, 0}; // the last tempo tap times
    
    int8_t liveVelocity = 0; // the velocity of the last input, used to set the velocity in live mode
    int8_t liveMode = 0; // set to record live input
    int8_t liveQuantize = 0; // set to quantize live input
    int8_t patternCursor = 0; // cursor position inside the pattern sequence
    int8_t patternSeqIndex = 0; // index into the pattern sequence we are currently playing
    int8_t patternMode = 0; // 0 = one pattern, 1 = sequence
    int8_t patternModeSwitch = 0; // set to indicate that the pattern should switch at the next mix!
    int8_t fillPattern = -1; // the pattern to fill at the end of this pattern. If non-zero, play that pattern before loop/advance
    int8_t lastPattern = 0; // pattern we were in before the fill
    int16_t nKits; // number of kits available (set at start)
    

    bool forceResynth; // if true, force the samples to be regenerated and not loaded from SD
    int8_t splashFlag; // are we currently showing the splash screen?
    int16_t lastMode; // last mode we were in (so we can return to it)
    int16_t bankAction; // next bank-select action to take (e.g. save, load, etc)
    String fileName; // the current file/bank name for the pattern
    cursor_t cursor;
    channel_t channels[nChans];
    chanData_t* currentPattern;
    chanData_t allPatterns[nSteps * nChans * maxPatterns];
    chanData_t clipboard[nSteps * nChans];
    previewData_t previewData;
    mixData_t mixData[nChans];
};





#endif

