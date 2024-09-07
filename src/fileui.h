#ifndef FILEUI_H
#define FILEUI_H

#include "patternui.h"
#include "datatypes.h"

void fileModeUpdate(DrumMachine &dm);
void fileModeKeys(DrumMachine &dm);
void setGraphicsModeFile(DrumMachine &dm);

typedef struct fileui_t
{
    String fileName;
    std::vector <String> files;
    std::vector <String> filteredFiles;
    int cursorPos;
} fileui_t;

#endif 