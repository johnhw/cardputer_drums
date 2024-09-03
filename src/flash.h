#ifndef FLASH_H
#define FLASH_H
#include <LittleFS.h>
#include <vector>

// functions for SPIFFS file system
void littlefsError(char *msg);
bool loadFile(const String &path, String &content);
bool saveFile(const String &path, const String &content) ;
std::vector<String> listFiles(const String &path);
bool createDirIfNotExists(const String path);
bool initLittleFS();



#endif 
