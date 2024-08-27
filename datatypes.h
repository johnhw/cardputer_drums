#ifndef DATATYPES_H
#define DATATYPES_H
#include <stdint.h>

// one channel
typedef struct channel_t
{
  int16_t volume;
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
} mixData_t;
#endif