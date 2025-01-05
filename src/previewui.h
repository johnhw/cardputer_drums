#include "config.h"
#include "datatypes.h"

void setGraphicsModePreview();
// in preview mode, just play samples immediately
void previewModeUpdate(DrumMachine &dm);
void previewSample(DrumMachine &dm, char sample);
void detuneSample(DrumMachine &dm, char sample, int32_t detune);
void resetDetuneSample(DrumMachine &dm, char sample);