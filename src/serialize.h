#ifndef SERIALIZE_H
#define SERIALIZE_H
#include "datatypes.h"
#include <LittleFS.h>

bool readDrumMachine(DrumMachine &dm, File &ser);
bool writeDrumMachine(DrumMachine &dm, File &ser);
bool writeSampleAdjustment(sample_adjustment_t *adj, File &ser);
bool readSampleAdjustment(sample_adjustment_t *adj, File &ser);
bool writeSampleAdjustments(DrumMachine &dm, File &ser);
bool readSampleAdjustments(DrumMachine &dm, File &ser);
bool saveDrumMachine(DrumMachine &dm, String &fname);
bool serialiseKit(DrumMachine &dm, File &ser);
bool deserialiseKit(DrumMachine &dm, File &ser);


#endif
