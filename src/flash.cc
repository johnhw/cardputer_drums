#include "flash.h"
#include <SPIFFS.h>
#include <M5Cardputer.h>
#include <FS.h>
#include <vector>

char spiffsErrorBuf[256];
void spiffsError(char *msg)
{
    snprintf(spiffsErrorBuf, sizeof(spiffsErrorBuf)-1, msg);
}

// Load a file from SPIFFS into a buffer
bool loadFile(const String &path, String &content) {
    if (!SPIFFS.begin()) {
        spiffsError("Failed to mount SPIFFS");
        return false;
    }

    File file = SPIFFS.open(path, FILE_READ);
    if (!file) {
        spiffsError("Failed to open file");
        return false;
    }

    content = file.readString();  // Read the entire file into a String
    file.close();
    return true;
}

// Save a buffer to a file in SPIFFS
bool saveFile(const String &path, const String &content) {
    if (!SPIFFS.begin()) {
        spiffsError("Failed to mount SPIFFS");
        return false;
    }

    File file = SPIFFS.open(path, FILE_WRITE);
    if (!file) {
        spiffsError("Failed to open file for writing");
        return false;
    }

    size_t written = file.print(content);
    if (written != content.length()) {
        spiffsError("Failed to write full data to file");
        file.close();
        return false;
    }

    file.close();
    return true;
}

// List all files in the root directory of SPIFFS
std::vector<String> listFiles(const String &path) {
    std::vector<String> fileList;
    fileList.clear();
    if (!SPIFFS.begin()) {
        spiffsError("Failed to mount SPIFFS");
        return fileList;
    }

    File root = SPIFFS.open(path);
    if (!root) {
        spiffsError("Failed to open directory");
        return fileList;
    }

    File file = root.openNextFile();
    while (file) {
        fileList.push_back(String(file.name()));
        file = root.openNextFile();
    }
}

// Create a directory if it does not exist
bool createDirIfNotExists(const String path) {
    if (!SPIFFS.begin()) {
        spiffsError("Failed to mount SPIFFS");
        return false;
    }

    // Check if the directory exists
    if (SPIFFS.exists(path)) {        
        return true;
    }

    // Try to create the directory
    if (SPIFFS.mkdir(path)) {        
        return true;
    } else {
        spiffsError("Failed to create directory");
        return false;
    }
}

// select strings with a given prefix
std::vector<String> filterByPrefix(const std::vector<String> &input, const String &prefix) {
    std::vector<String> result;

    for (const String &item : input) {
        if (item.startsWith(prefix)) {
            result.push_back(item);
        }
    }

    return result;
}