#include "config.h"
#include "datatypes.h"

void setGraphicsModePreview();
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
void initPreviewMode(DrumMachine &dm);
void recordAudioLoop(DrumMachine &dm);
void drawPreviewLoopMode(DrumMachine &dm);

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

#define MAX_SAMPLE_ADJUST 160000

const int32_t paramLimits[PARAM_N][2] = {
    {-2400, 2400},                           // detune
    {-2000, 2000},                           // volume
    {0, 1000},                               // cutoff
    {-MAX_SAMPLE_ADJUST, MAX_SAMPLE_ADJUST}, // trim start
    {0, MAX_SAMPLE_ADJUST},                  // trim end
    {0, MAX_SAMPLE_ADJUST},                  // loop start
    {0, MAX_SAMPLE_ADJUST},                  // loop end
    {0, 5000},                               // attack
    {0, 5000},                               // decay
    {-1000, 1000},                               // sustain
    {0, 5000}                                // release
};

const String paramNames[] = {
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
    "Release"};

enum SCALES
{
    SCALE_NORMAL,
    SCALE_FINE,
    SCALE_COARSE,
    SCALE_EXTRA_FINE,
    SCALE_EXTRA_COARSE
};
