#ifndef PATTERN_H
#define PATTERN_H
#include <stdint.h>
#include "datatypes.h"
#include "config.h"


void setCursorChar(DrumMachine& dm, char c);
void setCursorVel(DrumMachine& dm, char c);
void setCursorKick(DrumMachine& dm, int32_t kick);

void getCursorChar(DrumMachine& dm, char &ch, int16_t &velocity);
void clearPattern(DrumMachine& dm);
void _setPattern(DrumMachine& dm, int ix);
void updatePattern(DrumMachine& dm);
void resetPatternElement(chanData_t* elt);
void setPattern(DrumMachine& dm, int ix);
void copyPattern(DrumMachine& dm);
void pastePattern(DrumMachine& dm);
void selectiveClearPattern(DrumMachine& dm);

int32_t getKickDelay(DrumMachine& dm, int16_t step, int16_t chan);
sample_t *getSample(DrumMachine& dm, int16_t sampleIndex);
void getStep(DrumMachine& dm, int16_t step, int16_t chan, sample_t *&sample, int16_t &velocity);

#endif