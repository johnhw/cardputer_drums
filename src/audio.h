#ifndef AUDIO_H
#define AUDIO_H
#include <stdint.h>
#include "datatypes.h"
#include "synth.h"
#include "utils.h"


void mixPatternToBuffer(DrumMachine& dm, int16_t *buffer);
void resetMix(DrumMachine& dm);
void feedPatternBuffers(DrumMachine &dm);
void createSamples(DrumMachine& dm, kit_t &kit);
void allocateMix(DrumMachine& dm);
void allocSample(DrumMachine& dm, int8_t index, int32_t len);
bool renderToSD(DrumMachine &dm, String fname);
void autoSample(DrumMachine &dm, int drumIndex);
void previewSample(DrumMachine &dm, sample_t *preview);
void resetMix(DrumMachine &dm);
void noUINextPattern(DrumMachine &dm);
void clearSamples(DrumMachine &dm);
void allocSamples(DrumMachine &dm);
void recalcBPM(DrumMachine &dm);


#endif 