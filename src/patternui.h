
#ifndef PATTERNUI_H
#define PATTERNUI_H
#include <stdint.h>
#include <M5Cardputer.h>
#include "datatypes.h"
#include "config.h"

void updateCursor(DrumMachine &dm);
void patternModeKeys(DrumMachine &dm);
void patternKey(DrumMachine& dm, Keyboard_Class::KeysState status);
void fnKey(DrumMachine& dm, Keyboard_Class::KeysState status);
void setGraphicsModePattern();
void redrawPattern(DrumMachine& dm);
void drawStatus(DrumMachine& dm);
void drawCursor(DrumMachine& dm, int state);
void patternModeUpdate(DrumMachine& dm);
void adjSwing(DrumMachine& dm, int adj);
void adjBpm(DrumMachine& dm, int adj);
void adjCursorKick(DrumMachine& dm, int adj);
void moveCursor(DrumMachine& dm, int x, int y);
void drawKitLoading(DrumMachine& dm, int kit);
void drawTopLine(DrumMachine &dm);
void lowerMessage(DrumMachine &dm, const char *message);
void adjChanFilter(DrumMachine &dm, int adj);
void adjChanVolume(DrumMachine &dm, int adj);
  


#endif