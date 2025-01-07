#include "config.h"
#include "datatypes.h"

void setGraphicsModePreview();
// in preview mode, just play samples immediately
void previewModeUpdate(DrumMachine &dm);
void previewSample(DrumMachine &dm, char sample);
void redrawPreview(DrumMachine &dm);
void playPreviewSample(DrumMachine &dm, char sample);
void setKitParamValue(DrumMachine &dm, int32_t param, char sample, int32_t value);
void getKitParamValue(DrumMachine &dm, int32_t param, char sample, int32_t &value);
void prevKitParam(DrumMachine &dm);
void nextKitParam(DrumMachine &dm);
void adjustKitParam(DrumMachine &dm, int scaleMode, int adj);
void resetKitParam(DrumMachine &dm);


enum PARAMS 
{
    PARAM_DETUNE,
    PARAM_VOLUME,
    PARAM_CUTOFF,
    PARAM_TRIM_START,
    PARAM_TRIM_END,
    PARAM_LOOP_START,
    PARAM_LOOP_END,
    PARAM_ATTACK_TIME,
    PARAM_DECAY_TIME,
    PARAM_SUSTAIN_LEVEL,
    PARAM_RELEASE_TIME,
    PARAM_N
};

const String paramNames [] = {
    "Detune",
    "Volume",
    "Cutoff",
    "Start",
    "End",
    "Lp St",
    "Lp End",
    "Attack",
    "Decay",
    "Sustain",
    "Release"
};