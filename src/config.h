#ifndef CONFIG_H
#define CONFIG_H
#include <stdint.h>
#include <Arduino.h>
static constexpr const int32_t samplerate = 16000;
static constexpr const int16_t nSteps = 16;
static constexpr const int16_t nChans = 8;
static constexpr const int16_t maxPatterns = 36;
static constexpr const int16_t kickSubdiv = 12; // subdivisions per unit time
static constexpr const int16_t minBPM = 60;     // minimum bpm
static constexpr const int16_t cursorFlashTime = 200;
static constexpr const int16_t maxPatternSequence = 36;
static constexpr const int16_t maxFilterCutoff = 16; // number of steps in the filter cutoff
static constexpr const int16_t maxChannelVolume = 16; // number of steps in the channel volume setting
static constexpr const float maxSampleLen = 0.5; 
static constexpr const int32_t SAMPLE_ARENA_SIZE = 160000;
const String VERSION = "1.0.2";
const String basePathPattern = "/bonnethead/patterns"; // paths in the filesystem to save/load from
const String basePathKits = "/bonnethead/kits"; 
const String basePathSamples =  "/bonnethead/samples"; 
const String basePathRender = "/bonnethead/render";
const String basePathRoot = "/bonnethead"; 

#define N_BUFFERS 2

// pattern mode
#define PLAY_MODE_PATTERN 0 
#define PLAY_MODE_PREVIEW 1
#define PLAY_MODE_CONFIRM 2
#define PLAY_MODE_FILE 3
#define PLAY_MODE_KIT_EDIT 4
#define PLAY_MODE_HELP 5 

#define ACTION_NONE 0
#define ACTION_SAVE 1
#define ACTION_LOAD 2
#define ACTION_NEW 3


const String ASCIIDance[] = {
"(- - b)",
"(- - b>",
"([- -])",
"<{- -}>",
"(d - -)",
"<d - -)",
"([- -])",
"<{- -}>",
};

const char alphaNumericChars [] = "0123456789abcdefghijklmnopqrstuvwxyz";
const char shiftAlphaNumericChars [] = ")!@#$%^&*(ABCDEFGHIJKLMNOPQRSTUVWXYZ";
const char digitChars [] = "0123456789";
#endif