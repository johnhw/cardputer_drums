#ifndef SERIALIZE_H
#define SERIALIZE_H
#include <ArduinoJson.h>
#include "datatypes.h"
#include "config.h"

void serializeDrumMachine(DrumMachine &dm, String &json);
bool deserializeDrumMachine(DrumMachine &dm, String &json);


#endif 
