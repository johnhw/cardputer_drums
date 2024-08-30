#ifndef CONFIG_H
#define CONFIG_H
#include <stdint.h>
#include <Arduino.h>
static constexpr const int32_t samplerate = 16000;
static constexpr const int16_t nSteps = 16;
static constexpr const int16_t nChans = 8;
static constexpr const int16_t maxPatterns = 20;
static constexpr const int16_t kickSubdiv = 12; // subdivisions per unit time
static constexpr const int16_t minBPM = 60;     // minimum bpm
static constexpr const int16_t cursorFlashTime = 200;
static constexpr const int16_t maxPatternSequence = 32;
const String JSON_VERSION = "1.0.0";
const String basePath = "bonnethead"; // path in the filesystem to save/load from

// pattern mode
#define PLAY_MODE_PATTERN 0 
#define PLAY_MODE_PREVIEW 1
#define PLAY_MODE_CONFIRM 2
#define PLAY_MODE_FILE 3
#define PLAY_MODE_KIT_EDIT 4

#endif