#ifndef FLASH_H
#define FLASH_H
#include <LittleFS.h>
#include <vector>
#include "datatypes.h"
#include <M5GFX.h>

#define WAV_HEADER_LEN 44

// functions for SPIFFS file system
void littlefsError(char *msg);
bool loadFile(const String &path, String &content);
bool saveFile(const String &path, const String &content);
std::vector<String> listFiles(const String &path);
bool initLittleFS();
std::vector<String> filterByPrefix(const std::vector<String> &input, const String &prefix);
bool initSD();
bool createDirIfNotExistsSD(const String path);
bool openWAVToSD(String fname, int32_t samplerate);
bool appendWAVToSD(String fname, int16_t *audioData, size_t length);
bool backpatchWAVToSD(String fname);
bool validateWavHeader(byte *buffer);
String findFreeRenderFilenameSD();
bool writeWavSD(String fname, int32_t samplerate, int16_t *samples, int32_t len);
bool saveKitSD(const String &path, DrumMachine &dm);
bool loadKitSD(const String &path, DrumMachine &dm);

bool loadFont(M5GFX display, const String &path);

#endif
