#ifndef AUDIO_H
#define AUDIO_H
#include <stdint.h>
#include "datatypes.h"
#include "synth.h"


void mix(DrumMachine& dm);
void feedPatternBuffers(DrumMachine &dm);
void createSamples(DrumMachine& dm, kit_t &kit);
void allocateMix(DrumMachine& dm);
void allocSample(DrumMachine& dm, int8_t index, int32_t len);
bool renderToSD(DrumMachine &dm, String fname);
void autoSample(DrumMachine &dm, int drumIndex);
void freeSample(DrumMachine &dm, int8_t index);
void previewSample(DrumMachine &dm, sample_t *preview);
#endif 