#ifndef DATATYPES_H
#define DATATYPES_H
#include <stdint.h>
#include "config.h"

// one channel
typedef struct channel_t
{
  float volume;     // 0 - 16, 8 = 100%, each step is +/- 3dB
  int filterCutoff; // 0-16 16=no filter, 0=full filter, 1/16th of samplerate
  int8_t mute;
  int8_t solo;
  int8_t _enabled; // NB: not directly modified; updated to reflect mute/solo status of all channels
  int32_t detune;  // detune in cents
} channel_t;

typedef struct sample_adjustment_t
{
  int32_t detune = 0;    // detune in cents
  int32_t cutoff = 0;    // cutoff; +1000 cutoff = 0, 0 = SR
  int32_t volume = 0;    // signed, each step is 1/10th of a dB (centibels)
  int32_t trimStart = 0; // start of the sample (negative = delay)
  int32_t trimEnd = 0;   // offset from end of the sample (always positive)
  int32_t loopStart = 0; // start of the loop
  int32_t loopEnd = 0;   // end of the loop (if start==end, no loop)
  // envelope (TODO)
  int32_t attackTime = 0;   // attack time in ms
  int32_t decayTime = 0;    // decay time in ms
  int32_t sustainLevel = 0; // sustain level in 1/100th of a dB
  int32_t releaseTime = 0;  // release time in ms
  bool loopEnabled = false; // loop enabled
} sample_adjustment_t;

// a sample, with PCM data, length and a frequency
typedef struct sample_t
{
  int16_t *samples = 0;
  int32_t len = 0;
  sample_adjustment_t adjustments;
} sample_t;

enum FX
{
  FX_NONE,
  FX_FLAM,
  FX_ROLL,
  FX_REVERSE,
  FX_N
};

// one step of a channel
typedef struct chanData_t
{
  int16_t type;
  int16_t velocity;
  int32_t kickDelay; // additional delay, in kickSubdiv units
  int8_t fx;         // effect to apply
  int32_t detune;    // detune in cents
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
typedef struct arena_t
{
  void *start;
  void *end;
  void *top;
} arena_t;

typedef struct fxData
{
  bool reverse;
} fxData;

// data for mixing one channel into the final mix
typedef struct mixData_t
{
  sample_t *currentSample;       // pointer to current sample
  int32_t sampleIndex;           // current sample index
  int32_t fractionalSampleIndex; // in fractional samples (1/32768)
  int16_t stepIndex;             // current step index
  int16_t nextIndex;             // next step index
  int32_t kickDelay;             // pre-delay in samples
  int16_t currentVelocity;       // velocity for this step
  float totalGain;               // product of channel gain, step velocity and sample gain
  float channelGain;             // gain for this channel
  float currentFilter;           // current filter value
  float filterAlpha;             // filter alpha value
  int32_t totalDetune;           // total detune for this channel (sum of channel and sample detune)
  int32_t freqIncrement;         // frequency increment for the sample (32768.0 is a frequency of 1.0)
  int32_t channelDetune;         // detune for this channel
  int32_t channelCutoff;         // cutoff for this channel
  fxData fx;                     // current FX data
} mixData_t;

typedef struct previewData_t
{
  char lastSample = 0;      // last sample index (0-26)
  int16_t currentParam = 0; // current parameter being edited in preview mode
  bool previewDirty = true; // flag to indicate that the preview needs to be updated
  int32_t detune = 0;       // current detune value for the preview
  chanData_t previewChan;   // the current channel data for the preview (always dummy)
  mixData_t previewMix;     // the current mix data for the preview
} previewData_t;

struct DrumMachine
{
  sample_t drumSamples[27];
  int16_t *audioBuffers[N_BUFFERS]; // double buffering
  int32_t maxBufferLen;             // true (max) length of one buffer
  int16_t *bufferA, *bufferB;       // the two buffers
  arena_t sampleArena;              // memory arena for samples

  int32_t mixIndex = 0;       // current index inside the pattern (in samples)
  int8_t waveBufferIndex = 0; // current buffer count (4 buffers per bar)
  int32_t patternSamples;     // number of samples in a whole pattern
  int16_t waveBufferLen;      // samples in one buffer for a 1/4 pattern
  int16_t stepSamples;        // length of one step in samples (usually 1/16th of a pattern)
  int32_t oneKickTime;        // time in samples to kick forward by one kickSubdiv of a step (usually 1/12 of a step)

  int8_t playMode = 0; // 0=normal; 1=sample preview
  int16_t syncMix = 0; // flag to indicate to remix at the next pattern loop
  uint32_t syncMillis; // time in milliseconds of the last pattern loop start

  int16_t beatTime;   // the current beat/step we are in right now
  float playStep = 0; // the (fractional) step we are currently playing

  int16_t bpm = 120;
  int16_t swing = 0;
  int16_t pattern = 0;
  int16_t kit = 0;
  int16_t volume = 0; // 16 volume levels

  char patternSequence[maxPatternSequence];

  int64_t tapBufferMillis[4] = {0, 0, 0, 0}; // the last tempo tap times

  int8_t liveVelocity = 0;          // the velocity of the last input, used to set the velocity in live mode
  int8_t liveMode = 0;              // set to record live input
  int8_t liveQuantize = 0;          // set to quantize live input
  int8_t patternCursor = 0;         // cursor position inside the pattern sequence
  int8_t patternSeqIndex = 0;       // index into the pattern sequence we are currently playing
  int8_t patternMode = 0;           // 0 = one pattern, 1 = sequence
  int8_t patternModeSwitch = 0;     // set to indicate that the pattern should switch at the next mix!
  int8_t fillPattern = -1;          // the pattern to fill at the end of this pattern. If non-zero, play that pattern before loop/advance
  int8_t lastPatternBeforeFill = 0; // pattern we were in before the fill
  int16_t nKits;                    // number of kits available (set at start)

  int32_t lastMessageMillis; // number of milliseconds since the last message was displayed
  int8_t splashFlag;         // are we currently showing the splash screen?
  int16_t lastMode;          // last mode we were in (so we can return to it)
  int16_t bankAction;        // next bank-select action to take (e.g. save, load, etc)

  cursor_t cursor;
  channel_t channels[nChans];

  chanData_t *currentPattern;
  chanData_t allPatterns[nSteps * nChans * maxPatterns];
  chanData_t clipboard[nSteps * nChans];
  previewData_t previewData;
  mixData_t mixData[nChans];
};

#endif
