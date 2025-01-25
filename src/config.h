#ifndef CONFIG_H
#define CONFIG_H
#include <stdint.h>
#include <Arduino.h>

static constexpr const int32_t samplerate = 16000;         // sample rate in Hz
static constexpr const int16_t nSteps = 16;                // number of steps in a pattern
static constexpr const int16_t nChans = 8;                 // number of channels
static constexpr const int16_t maxPatterns = 36;           // maximum number of patterns
static constexpr const int16_t kickSubdiv = 12;            // subdivisions per unit time
static constexpr const int16_t minBPM = 60;                // minimum bpm
static constexpr const int16_t cursorFlashTime = 200;      // time in ms for the cursor to flash
static constexpr const int16_t maxPatternSequence = 36;    // maximum length of the pattern sequence
static constexpr const int16_t maxFilterCutoff = 16;       // number of steps in the filter cutoff
static constexpr const int16_t maxChannelVolume = 16;      // number of steps in the channel volume setting
static constexpr const float maxSampleLen = 0.5;           // maximum length in seconds of a sample
static constexpr const int32_t SAMPLE_ARENA_SIZE = 160000; // number of samples in the kit arena (all samples are consecutive slices of this)
static constexpr const int32_t bufferBeats = 8;            // division of a bar in one audio buffer (4 = 1/4 bar)
static constexpr const int32_t messageTime = 1000;         // time in ms to show a status message
static constexpr const int32_t maxSamples = 27;           // maximum number of samples in a kit

const String VERSION = "1.0.6";
const String basePathPattern = "/bonnethead/patterns"; // paths in the filesystem to save/load from
const String basePathKits = "/bonnethead/kits";
const String basePathSamples = "/bonnethead/samples";
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

extern const String ASCIIDance[];

const char alphaNumericChars[] = "0123456789abcdefghijklmnopqrstuvwxyz";
const char shiftAlphaNumericChars[] = ")!@#$%^&*(ABCDEFGHIJKLMNOPQRSTUVWXYZ";
const char digitChars[] = "0123456789";
const char shiftDigitChars[] = ")!@#$%^&*(";

// keyboard layout (piano style)
const char keyboardSemitoneSequence[] = "zsxdcvgbhnjmq2w3er5t6y7ui9o0p";
const char shiftKeyboardSemitoneSequence[] = "ZSXDCVGBHNJMQ@W#ER%T^Y&UI*O)P";

// names for the status display
extern const char* noteNames [];
extern const char *fxNames [];

// logit spaced probabilities for notes
extern const float probabilityTable[];



#endif