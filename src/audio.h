#ifndef AUDIO_H
#define AUDIO_H
#include <stdint.h>
#include "datatypes.h"
#include "synth.h"
#include "utils.h"
#include "adsr.h"

void mixPatternToBuffer(DrumMachine &dm, int16_t *buffer, int32_t len);
void resetMix(DrumMachine& dm);
void triggerPreviewSample(DrumMachine &dm, int index);
void mixSingleSampleToBuffer(DrumMachine &dm, int16_t *buffer, int32_t len);
void mixPatternToBuffer(DrumMachine &dm, int16_t *buffer, int32_t len);
void mixTriggerSample(DrumMachine &dm, mixData_t *mx, chanData_t *ch);

void feedPatternBuffers(DrumMachine &dm);
void feedPreviewBuffers(DrumMachine &dm);
void synthKitSamples(DrumMachine& dm, kit_t &kit);
void allocateMix(DrumMachine& dm);
bool renderToSD(DrumMachine &dm, String fname);
void previewSample(DrumMachine &dm, sample_t *preview);
void resetMix(DrumMachine &dm);
void noUINextPattern(DrumMachine &dm);
void clearSamples(DrumMachine &dm);
void allocSamples(DrumMachine &dm);
void recalcBPM(DrumMachine &dm);
void resetAudioPlayback(DrumMachine &dm);

#endif 