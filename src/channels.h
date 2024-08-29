#ifndef CHANNELS_H
#define CHANNELS_H
#include <stdint.h>
#include "datatypes.h"
#include "config.h"

int recalcChannels(DrumMachine& dm);
void toggleSolo(DrumMachine& dm, int ix);
void toggleMute(DrumMachine& dm, int ix);

#endif