#ifndef UI_H
#define UI_H

#include <stdint.h>
#include "datatypes.h"
#include "config.h"

void requestMix(DrumMachine& dm);
void recalcBPM(DrumMachine& dm);
void resetState(DrumMachine& dm);
void updatePattern(DrumMachine& dm);
void setPlayMode(DrumMachine &dm, int mode);
void updateMix(DrumMachine& dm, int16_t step, int16_t chan);
void setKit(DrumMachine& dm, int kit);
void requestMix(DrumMachine& dm);
void nextPattern(DrumMachine &dm);

#endif