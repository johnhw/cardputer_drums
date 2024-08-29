#include "channels.h"
#include "ui.h"
#include "patternui.h"
#include "ui.h"

// update the enabled flag across channels
// using the solo and mute information
// returns the number of solo'd channels
int recalcChannels(DrumMachine& dm) {
    int solo = 0;
    for (int i = 0; i < nChans; i++) {
        solo += dm.channels[i].solo;
    }
    if (solo == 0) { // no solo, enable all
        for (int i = 0; i < nChans; i++) {
            dm.channels[i]._enabled = 1;
        }
    } else {
        // select only solo'd channels
        for (int i = 0; i < nChans; i++) {
            dm.channels[i]._enabled = dm.channels[i].solo;
        }
    }

    // now apply mutes
    for (int i = 0; i < nChans; i++) {
        if (dm.channels[i].mute)
            dm.channels[i]._enabled = 0;
    }
    return solo;
}

void toggleSolo(DrumMachine& dm, int ix) {
    if (ix < 0 || ix >= nChans)
        return;
    dm.channels[ix].solo = !dm.channels[ix].solo;
    recalcChannels(dm);
    drawStatus(dm);
    updateMix(dm, 0, 0);
}

void toggleMute(DrumMachine& dm, int ix) {
    if (ix < 0 || ix >= nChans)
        return;
    dm.channels[ix].mute = !dm.channels[ix].mute;
    recalcChannels(dm);
    drawStatus(dm);
    updateMix(dm, 0, 0);
}
