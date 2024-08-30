#ifndef FLASH_H
#define FLASH_H
#include <SPIFFS.h>
#include <vector>

// functions for SPIFFS file system
void spiffsError(char *msg);
bool loadFile(const String &path, String &content);
bool saveFile(const String &path, const String &content) ;
std::vector<String> listFiles(const String &path);
bool createDirIfNotExists(const String path);


#endif 
