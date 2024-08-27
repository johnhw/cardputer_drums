#ifndef CONFIG_H
#define CONFIG_H
static constexpr const size_t samplerate = 16000;
static constexpr const int16_t nSteps = 16;
static constexpr const int16_t nChans = 8;
static constexpr const int16_t maxPatterns = 10;
static constexpr const int16_t kickSubdiv = 12; // subdivisions per unit time
static constexpr const int16_t minBPM = 60;     // minimum bpm
static constexpr const int16_t cursorFlashTime = 200;
#endif