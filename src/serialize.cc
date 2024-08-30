// stores and retrieves DrumMachine data
#include "serialize.h"

#define JSON_SIZE 256

void serializeDrumMachine(DrumMachine &dm, String &json) {
    StaticJsonDocument<JSON_SIZE> doc;

    // Version number (incompatible files must use a different version number!)
    doc["version"] = JSON_VERSION;

    // global settings
    doc["bpm"] = dm.bpm;
    doc["swing"] = dm.swing;
    doc["pattern"] = dm.pattern;
    doc["kit"] = dm.kit;
    doc["volume"] = dm.volume;
    
    // the sequence of patterns
    doc["patternSequence"] = String(dm.patternSequence);
    
    
    // channel states
    JsonArray channels = doc["channels"].to<JsonArray>();
    for (int i = 0; i < nChans; i++) {
        JsonObject channel = channels.createNestedObject();
        channel["volume"] = dm.channels[i].volume;
        channel["mute"] = dm.channels[i].mute;
        channel["solo"] = dm.channels[i].solo;
        channel["filterCutoff"] = dm.channels[i].filterCutoff;
        channel["_enabled"] = dm.channels[i]._enabled;
    }

    // all patterns (i.e. the drum data)
    JsonArray allPatterns = doc["allPatterns"].to<JsonArray>();
    for (int i = 0; i < nSteps * nChans * maxPatterns; i++) {
        JsonObject pattern = allPatterns.createNestedObject();
        pattern["type"] = dm.allPatterns[i].type;        
        pattern["velocity"] = dm.allPatterns[i].velocity;
        pattern["kickDelay"] = dm.allPatterns[i].kickDelay;
    }

    // Convert JSON document to a string    
    serializeJson(doc, json);
}

bool deserializeDrumMachine(DrumMachine &dm, String &json) {
    StaticJsonDocument<JSON_SIZE> doc;

    // Parse the JSON object
    DeserializationError error = deserializeJson(doc, json);
    if (error) {
        return false;
    }

    // must match the version
    if(doc["version"] != JSON_VERSION) {
        return false;
    }

    dm.bpm = doc["bpm"];
    dm.swing = doc["swing"];
    dm.pattern = doc["pattern"];
    dm.kit = doc["kit"];
    dm.volume = doc["volume"];

    doc["patternSequence"].as<String>().toCharArray(dm.patternSequence, maxPatternSequence);    
    
    
    JsonArray channels = doc["channels"].as<JsonArray>();
    for (int i = 0; i < nChans; i++) {
        dm.channels[i].volume = channels[i]["volume"];
        dm.channels[i].mute = channels[i]["mute"];
        dm.channels[i].solo = channels[i]["solo"];
        dm.channels[i].filterCutoff = channels[i]["filterCutoff"];
        dm.channels[i]._enabled = channels[i]["_enabled"];
    }

    JsonArray allPatterns = doc["allPatterns"].as<JsonArray>();
    for (int i = 0; i < nSteps * nChans * maxPatterns; i++) {
        dm.allPatterns[i].type = allPatterns[i]["type"];
        dm.allPatterns[i].velocity = allPatterns[i]["velocity"];
        dm.allPatterns[i].kickDelay = allPatterns[i]["kickDelay"];
    }
    
    return true;
}