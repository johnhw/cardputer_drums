#ifndef AUDIO_H
#define AUDIO_H
#include <stdint.h>
#include "datatypes.h"



void mix(DrumMachine& dm);
void feedPatternBuffers(DrumMachine &dm);
void createSamples(DrumMachine& dm);
void allocateMix(DrumMachine& dm);
void allocSample(DrumMachine& dm, int8_t index, int32_t len);


#endif 