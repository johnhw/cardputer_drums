#ifndef CONFIG_H
#define CONFIG_H
static constexpr const int32_t samplerate = 16000;
static constexpr const int16_t nSteps = 16;
static constexpr const int16_t nChans = 8;
static constexpr const int16_t maxPatterns = 20;
static constexpr const int16_t kickSubdiv = 12; // subdivisions per unit time
static constexpr const int16_t minBPM = 60;     // minimum bpm
static constexpr const int16_t cursorFlashTime = 200;
static constexpr const int16_t maxPatternSequence = 32;
#endif