#ifndef SERIALIZE_H
#define SERIALIZE_H
#include <ArduinoJson.h>
#include "datatypes.h"
#include <SPIFFS.h>

bool readDrumMachine(DrumMachine &dm, File &ser);
bool writeDrumMachine(DrumMachine &dm, File &ser);


#endif 
