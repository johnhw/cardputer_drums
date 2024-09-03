#include "flash.h"

#include <M5Cardputer.h>
#include <vector>



#define FORMAT_SPIFFS_IF_FAILED true

char LittleFSErrorBuf[256];
void LittleFSError(char *msg)
{
    snprintf(LittleFSErrorBuf, sizeof(LittleFSErrorBuf)-1, msg);
    Serial.println(LittleFSErrorBuf);
}

bool initLittleFS() {
    if (!LittleFS.begin(FORMAT_SPIFFS_IF_FAILED)) {
        LittleFSError("Failed to mount SPIFFS");
        return false;
    }
    return true;
}



bool loadFile(const String &path, String &content) {
    
    File file = LittleFS.open(path, FILE_READ);
    if (!file) {
        LittleFSError("Failed to open file for reading");
        return false;
    }

    content = file.readString();
    file.close();
    Serial.printf("Loaded file: %s\n", path.c_str());
    return true;
}

// Save a buffer to a file in SPIFFS
bool saveFile(const String &path, const String &content) {
    
    File file = LittleFS.open(path, "w");
    if (!file) {
        LittleFSError("Failed to open file for writing");
        return false;
    }

    size_t written = file.print(content);
    if (written != content.length()) {
        LittleFSError("Failed to write full data to file");
        file.close();
        return false;
    }

    Serial.println("Saved file: " + path);
    file.close();

    // now open it and verify the contents
    String verify;
    file = LittleFS.open(path, FILE_READ);
    if (!file) {
        LittleFSError("Failed to open file for verification");
        return false;
    }
    verify = file.readString();
    file.close();
    // check length
    if (verify.length() != content.length()) {
        LittleFSError("Failed to verify file length");
        Serial.printf("Expected: %d, got: %d\n", content.length(), verify.length());
        return false;
    }
    if (verify != content) {
        LittleFSError("Failed to verify file contents");
        return false;
    }
    
    return true;
}

// List all files in the root directory of SPIFFS
std::vector<String> listFiles(const String &path) {
    std::vector<String> fileList;
    fileList.clear();
    
    File root = LittleFS.open(path);
    if (!root) {
        LittleFSError("Failed to open directory");
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
    
    // Check if the directory exists
    if (LittleFS.exists(path)) {        
        
        return true;
    }

    // Try to create the directory
    if (LittleFS.mkdir(path)) {        
        
        return true;
    } else {
        LittleFSError("Failed to create directory");
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