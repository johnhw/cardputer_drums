#include "flash.h"
#include <SPIFFS.h>
#include <M5Cardputer.h>
#include <FS.h>
#include <vector>
#define FORMAT_SPIFFS_IF_FAILED true

char spiffsErrorBuf[256];
void spiffsError(char *msg)
{
    snprintf(spiffsErrorBuf, sizeof(spiffsErrorBuf)-1, msg);
    Serial.println(spiffsErrorBuf);
}

bool initSPIFFS() {
    if (!SPIFFS.begin(FORMAT_SPIFFS_IF_FAILED)) {
        spiffsError("Failed to mount SPIFFS");
        return false;
    }
    return true;
}



bool loadFile(const String &path, String &content) {
    Serial.printf("Loading file: %s\n", path.c_str());
    File file = SPIFFS.open(path, FILE_READ);
    if (!file) {
        spiffsError("Failed to open file for reading");
        return false;
    }

    content = file.readString();
    file.close();
    Serial.printf("Loaded file: %s\n", path.c_str());
    return true;
}

// Save a buffer to a file in SPIFFS
bool saveFile(const String &path, const String &content) {
    Serial.printf("Saving file: %s\n", path.c_str());
    File file = SPIFFS.open(path, "w");
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

    Serial.println("Saved file: " + path);
    file.close();

    // now open it and verify the contents
    String verify;
    file = SPIFFS.open(path, FILE_READ);
    if (!file) {
        spiffsError("Failed to open file for verification");
        return false;
    }
    verify = file.readString();
    file.close();
    // check length
    if (verify.length() != content.length()) {
        spiffsError("Failed to verify file length");
        Serial.printf("Expected: %d, got: %d\n", content.length(), verify.length());
        return false;
    }
    if (verify != content) {
        spiffsError("Failed to verify file contents");
        return false;
    }
    Serial.println("Verified file contents");
    return true;
}

// List all files in the root directory of SPIFFS
std::vector<String> listFiles(const String &path) {
    std::vector<String> fileList;
    fileList.clear();
    
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
    Serial.println("Creating directory: " + path);
    
    // Check if the directory exists
    if (SPIFFS.exists(path)) {        
        Serial.println("Path exists");
        return true;
    }

    // Try to create the directory
    if (SPIFFS.mkdir(path)) {        
        Serial.println("Made path");
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