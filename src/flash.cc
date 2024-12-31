#include "flash.h"
#include <M5Cardputer.h>
#include <vector>
#include <SD.h>
#include <SPI.h>
#include "audio.h"

#define FORMAT_SPIFFS_IF_FAILED true

char LittleFSErrorBuf[256];
void LittleFSError(char *msg)
{
    snprintf(LittleFSErrorBuf, sizeof(LittleFSErrorBuf) - 1, msg);
    Serial.println(LittleFSErrorBuf);
}

bool initLittleFS()
{
    if (!LittleFS.begin(FORMAT_SPIFFS_IF_FAILED))
    {
        LittleFSError("Failed to mount SPIFFS");
        return false;
    }
    return true;
}

bool loadFile(const String &path, String &content)
{

    File file = LittleFS.open(path, FILE_READ);
    if (!file)
    {
        LittleFSError("Failed to open file for reading");
        return false;
    }

    content = file.readString();
    file.close();
    Serial.printf("Loaded file: %s\n", path.c_str());
    return true;
}

// Save a buffer to a file in SPIFFS
bool saveFile(const String &path, const String &content)
{

    File file = LittleFS.open(path, "w");
    if (!file)
    {
        LittleFSError("Failed to open file for writing");
        return false;
    }

    size_t written = file.print(content);
    if (written != content.length())
    {
        LittleFSError("Failed to write full data to file");
        file.close();
        return false;
    }

    Serial.println("Saved file: " + path);
    file.close();

    // now open it and verify the contents
    String verify;
    file = LittleFS.open(path, FILE_READ);
    if (!file)
    {
        LittleFSError("Failed to open file for verification");
        return false;
    }
    verify = file.readString();
    file.close();
    // check length
    if (verify.length() != content.length())
    {
        LittleFSError("Failed to verify file length");
        Serial.printf("Expected: %d, got: %d\n", content.length(), verify.length());
        return false;
    }
    if (verify != content)
    {
        LittleFSError("Failed to verify file contents");
        return false;
    }

    return true;
}

// List all files in the root directory of SPIFFS
std::vector<String> listFiles(const String &path)
{
    std::vector<String> fileList;
    fileList.clear();

    File root = LittleFS.open(path);
    if (!root)
    {
        LittleFSError("Failed to open directory");
        return fileList;
    }

    File file = root.openNextFile();
    while (file)
    {
        fileList.push_back(String(file.name()));
        file = root.openNextFile();
    }
}

// select strings with a given prefix
std::vector<String> filterByPrefix(const std::vector<String> &input, const String &prefix)
{
    std::vector<String> result;

    for (const String &item : input)
    {
        if (item.startsWith(prefix))
        {
            result.push_back(item);
        }
    }

    return result;
}

bool initSD()
{
    if (!SD.begin())
    {
        Serial.println("SD Card initialization failed!");
        return false;
    }

    // create the directories if they don't exist
    createDirIfNotExistsSD(basePathRoot);
    createDirIfNotExistsSD(basePathPattern);
    createDirIfNotExistsSD(basePathKits);
    createDirIfNotExistsSD(basePathSamples);
    createDirIfNotExistsSD(basePathRender);
    return true;
}

// Create a directory if it does not exist
bool createDirIfNotExistsSD(const String path)
{

    // Check if the directory exists
    if (SD.exists(path))
    {
        return true;
    }

    // Try to create the directory
    if (SD.mkdir(path))
    {
        return true;
    }
    else
    {
        Serial.println("Failed to create directory");
        return false;
    }
}

bool listKitsSD(std::vector<String> &kits)
{
    File root = SD.open(basePathKits);
    if (!root)
    {
        Serial.println("Failed to open kits directory");
        return false;
    }
    File file = root.openNextFile();
    while (file)
    {
        kits.push_back(String(file.name()));
        file = root.openNextFile();
    }
    return true;
}


#define WAV_HEADER_LEN 44

/* Validate that the 44 byte header is a valid wav file
    mono, 16 bit, any sample rate */
bool validateWavHeader(byte *buffer)
{
    // check fixed header portion
    if (buffer[0] != 'R' || buffer[1] != 'I' || buffer[2] != 'F' || buffer[3] != 'F')
    {
        Serial.println("Invalid WAV header: not RIFF");
        return false;
    }
    if (buffer[22] != 1)
    {
        Serial.println("Invalid WAV header: not mono");
        return false;
    }
    if (buffer[34] != 16)
    {
        Serial.println("Invalid WAV header: not 16 bit");
        return false;
    }
    return true;
}

/* Write out each drum sample to a file on the SD card
   named a.wav, b.wav, ..., z.wav */
bool writeKitSD(const String &path, DrumMachine &dm)
{
    Serial.println("Writing kit: " + path);
    createDirIfNotExistsSD(path);
    // iterate over a.wav through z.wav
    for (int i = 0; i < 26; i++)
    {
        String fname = String((char)('a' + i)) + ".wav";
        String fullPath = path + "/" + fname;
        Serial.println("Writing kit sample: " + fullPath);
        bool success = writeWavSD(fullPath, samplerate, dm.drumSamples[i].samples, dm.drumSamples[i].len);

        if (!success)
        {
            Serial.println("Failed to open kit file for writing: " + fullPath);
            continue;
        }
        else
        {
            Serial.println("Wrote sample: " + fullPath);
        }
    }
    Serial.println("Wrote kit");
    return true;
}

/* Replace all samples in a drum machine with loaded
files from an SD card, assuming the files are named
a.wav, b.wav, ..., z.wav */
bool loadKitSD(const String &path, DrumMachine &dm)
{
    int oldKit = dm.kit;
    bool foundSamples = false;
    // iterate over a.wav through z.wav
    for (int i = 0; i < 26; i++)
    {
        String fname = String((char)('a' + i)) + ".wav";
        String fullPath = path + "/" + fname;

        File file = SD.open(fullPath, FILE_READ);
        // clear existing sample
        memset(dm.drumSamples[i].samples, 0, dm.drumSamples[i].len);
        dm.drumSamples[i].len = 0;                

        // skip missing files
        if (!file)
        {
            Serial.println("Failed to open file for reading: " + fullPath);
            continue;
        }

        // read the sample
        int16_t len = file.size();
        int16_t *buffer = dm.audioBuffers[0];
        // make len maximum the size of the buffer
        if (len > dm.waveBufferLen + WAV_HEADER_LEN)
        {
            len = dm.waveBufferLen + WAV_HEADER_LEN;
        }

        // read the header into a buffer
        byte header[WAV_HEADER_LEN];
        if (file.read(header, WAV_HEADER_LEN) != WAV_HEADER_LEN)
        {
            Serial.println("Failed to read WAV header");
            continue;
        }
        bool wav_ok = validateWavHeader(header);
        if (!wav_ok)
        {
            Serial.println("Invalid WAV header");
            continue;
        }
        if (file.read((byte *)buffer, len) != len - 44)
        {
            Serial.println("Failed to read sample data");
            continue;
        }
        autoSample(dm, i); // copy from wavebuffer[0] to drumSamples[i]
        foundSamples = true;
        file.close();
    }

    memset(dm.audioBuffers[0], 0, dm.waveBufferLen);
    return foundSamples;
}

/* Scan the /render folder, and find sequential filenames named render-00000.wav, render-00001.wav, etc.
   Keep scanning until a free filename is found. Return the first free filename */
String findFreeRenderFilenameSD()
{
    String path = basePathRender;
    bool root = SD.exists(path);
    if (!root)
    {
        Serial.println("Failed to open render directory");
        return "";
    }
    int i = 0;
    String fname;
    while (true)
    {
        fname = path + "/" + String("render-") + String(i, 5) + ".wav";
        if (!SD.exists(fname))
        {
            return fname;
        }
        i++;
    }
}

void write_uint32t(File &file, uint32_t val)
{
    file.write((byte)(val & 0xFF));
    file.write((byte)((val >> 8) & 0xFF));
    file.write((byte)((val >> 16) & 0xFF));
    file.write((byte)((val >> 24) & 0xFF));
}

/* Write a header to a file, with blank length fields, which can then be
append to and backpatched later */
bool openWAVToSD(String fname, int32_t samplerate)
{

    if (SD.exists(fname))
    {
        SD.remove(fname);
    }
    File file = SD.open(fname, FILE_WRITE);

    if (!file)
    {
        Serial.println("Failed to open WAV header file for writing.");
        return false;
    }

    // WAV header fields, with placeholders for chunk size and data size
    uint32_t byte_rate = 2 * samplerate; // byte rate (sample rate * bytes per sample)
    byte wavHeader[44] = {
        0x52, 0x49, 0x46, 0x46, // "RIFF"
        0xDE,                   // Chunk size (little-endian)
        0xAD,
        0xBE,
        0xEF,
        0x57, 0x41, 0x56, 0x45, // "WAVE"
        0x66, 0x6D, 0x74, 0x20, // "fmt "
        0x10, 0x00, 0x00, 0x00, // Subchunk1 size (16 for PCM)
        0x01, 0x00,             // Audio format (1 for PCM)
        0x01, 0x00,             // Number of channels (1 for mono)
        (samplerate & 0xFF),    // Sample rate
        ((samplerate >> 8) & 0xFF),
        ((samplerate >> 16) & 0xFF),
        ((samplerate >> 24) & 0xFF),
        (byte_rate & 0xFF), // Byte rate (SampleRate * NumChannels * BitsPerSample/8)
        ((byte_rate >> 8) & 0xFF),
        ((byte_rate >> 16) & 0xFF),
        ((byte_rate >> 24) & 0xFF),
        0x02, 0x00,             // Block align (NumChannels * BitsPerSample/8)
        0x10, 0x00,             // Bits per sample (16 bits)
        0x64, 0x61, 0x74, 0x61, // "data"
        0xCA,
        0xFE,
        0xBA,
        0xBE, // Subchunk2 size (data size)
    };

    // Write WAV header to file
    file.write(wavHeader, 44);
    // Close the file
    file.close();
    return true;
}

/* Add a block of audio data to the end of a WAV file */
bool appendWAVToSD(String fname, int16_t *audioData, size_t length)
{

    File file = SD.open(fname, FILE_APPEND);

    if (!file)
    {
        Serial.println("Failed to open WAV file for appending.");
        return false;
    }

    // Write audio data to file
    file.write((byte *)audioData, length*2);
    file.flush();

    // Close the file
    file.close();
    return true;
}

/* Open a file; get its length; and use this to
backpatch the chuck size and data size fields in the
WAV header */
bool backpatchWAVToSD(String fname)
{

    File file = SD.open(fname, FILE_READ);

    if (!file)
    {
        Serial.println("Failed to open file for reading.");
        return false;
    }

    // Get the file size
    uint32_t fileSize = file.size();

    if (fileSize < 44)
    {
        Serial.println("File too small to be a valid WAV file");
        return false;
    }

    file.close();
    file = SD.open(fname, "r+");
    uint32_t realFileSize = fileSize - 8;   // file size minus 8 bytes for "RIFF" and size fields
    uint32_t dataChunkSize = fileSize - 44; // size of the data chunk

    // Backpatch the chunk size and data size fields in the WAV header
    file.seek(4);
    write_uint32t(file, realFileSize);

    file.seek(40);
    write_uint32t(file, dataChunkSize);

    // Close the file
    file.close();

    // verify file size and print it out
    file = SD.open(fname, FILE_READ);
    if (!file)
    {
        Serial.println("Failed to open file for verification");
        return false;
    }
    fileSize = file.size();
    file.close();
    return true;
}

bool writeWavSD(String fname, int32_t samplerate, int16_t *samples, int32_t len)
{
    bool success;
    success = openWAVToSD(fname, samplerate);
    if (!success)
    {
        return false;
    }
    success = appendWAVToSD(fname, samples, len);
    if (!success)
    {
        return false;
    }
    success = backpatchWAVToSD(fname);
    if (!success)
    {
        return false;
    }
    return true;
}
